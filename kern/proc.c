#include <types.h>
#include <proc.h>
#include <param.h>
#include <string.h>
#include <memlayout.h>
#include <list.h>
#include <console.h>
#include <mm.h>
#include <vm.h>
#include <spinlock.h>
#include <vfs.h>
#include <filedesc.h>
#include <debug.h>
#include <usb.h>
#include <net/net.h>
#include <config.h>
#include <slab.h>
#include <mmap.h>
#include <linux/signal.h>
#include <linux/resources.h>
#include <linux/wait.h>
#include <linux/errno.h>
#include <linux/ppoll.h>

extern int sd_postinit(void);

extern void trapret();
extern void swtch(struct context **old, struct context *new);

static void forkret();
static void idle_init();

#define SQSIZE  0x100           /* Must be power of 2. */
#define HASH(x) ((((uint64_t)(x)) >> 5) & (SQSIZE - 1))

struct cpu cpu[NCPU];

struct {
    struct proc proc[NPROC];            /* proc/kthread */
    struct list_head slpque[SQSIZE];    /* sleep queue, chan毎に存在 */
    struct list_head sched_que;         /* runnable queue */
    struct spinlock lock;               /* 以上を保護するロック */
} ptable;

/* シグナル処理を行う際に使用する2つのロック */
struct _q {
    struct spinlock lock;
    struct spinlock siglock;
} q;

struct proc *initproc;
struct slab_cache *VMA;

/* ワークキュー　*/
static struct workqueue wq;

static int procid = 0;

void
proc_init(void)
{
    int i;

    initlock(&ptable.lock, "ptable");
    initlock(&q.lock, "q_lock");
    initlock(&q.siglock, "q_siglock");
    list_init(&ptable.sched_que);
    for (i = 0; i < SQSIZE; i++)
        list_init(&ptable.slpque[i]);
    for (i = 0; i < NPROC; i++)
        initlock(&(ptable.proc[i].time_lock), "time lock");
    VMA = slab_cache_create("vma", sizeof(struct vma), 0);

    info("proc_init ok");
}

// TODO: use kmalloc
/*
 * Look in the process table for an UNUSED proc.
 * If found, change state to EMBRYO and initialize
 * state required to run in the kernel.
 * Otherwise return 0.
 */
static struct proc *
proc_alloc(void)
{
    struct proc *p;
    int found = 0;

    acquire(&ptable.lock);
    for (p = ptable.proc; p < ptable.proc + NPROC; p++) {
        if (p->state == UNUSED) {
            memset(p, 0, sizeof(*p));
            found = 1;
            break;
        }
    }

    if (!found || !(p->kstack = kalloc())) {
        release(&ptable.lock);
        return 0;
    }

    p->pid = ++procid;
    p->state = EMBRYO;
    release(&ptable.lock);

    p->name[0] = 0;

    void *sp = p->kstack + PGSIZE;
    assert(sizeof(*p->tf) == 19 * 16 && sizeof(*p->context) == 7 * 16);

    sp -= sizeof(*p->tf);
    p->tf = sp;
    /* No user stack for init process. */
    p->tf->spsr = p->tf->sp = 0;

    sp -= sizeof(*p->context);
    p->context = sp;
    p->context->lr0 = (uint64_t) forkret;
    p->context->lr = (uint64_t) trapret;

    p->paused = 0;

    list_init(&p->child);
    trace("ksp: %p", sp);

    return p;
}

static struct proc *
proc_initx(char *name, char *code, size_t len)
{
    struct proc *p = proc_alloc();
    void *va = kalloc();
    assert(p && va);

    p->pgdir = vm_init();
    assert(p->pgdir);

    int ret = uvm_map(p->pgdir, 0, PGSIZE, V2P(va));
    assert(ret == 0);

    memmove(va, code, len);
    assert(len <= PGSIZE);

    // Flush dcache to memory so that icache can retrieve the correct one.
    dccivac(va, len);

    p->stksz = 0;
    p->sz = PGSIZE;
    p->base = 0;

    p->pgid = p->sid = p->pid;
    p->uid = p->gid = 0;
    p->fdflag = 0;
    init_fd_table(p->fd_table);
    p->umask = 0002;
    p->vmas = NULL;

    p->stime = p->utime = 0;
    p->it_real_value = p->it_real_incr = 0;
    memset(&p->real_timer, 0, sizeof(struct timer_list));

    p->cap_effective = p->cap_inheritable = p->cap_permitted = CAP_INIT_EFF_SET;

    p->tf->elr = 0;

    p->userdata = 0;

    safestrcpy(p->name, name, sizeof(p->name));
    return p;
}

/* Initialize per-cpu idle process. */
// TODO: idle関数を変更する
static void
idle_init(void)
{
    extern char ispin[], eicode[];
    thiscpu()->idle = proc_initx("idle", ispin, (size_t)(eicode - ispin));
}

/* Set up the first user process. */
void
user_init(void)
{
    extern char icode[], eicode[];

    trace("user_init start: icode = 0x%x, size = 0x%x", icode, (size_t)(eicode - icode));
    struct proc *p = proc_initx("icode", icode, (size_t)(eicode - icode));
    trace("p->pid = %d", p->pid);
    p->cwd = vfs_clone_vnode(get_rootfs()->root_node);
    trace("p->cwd->ino = %d", p->cwd->ino);

    acquire(&ptable.lock);
    list_push_back(&ptable.sched_que, &p->link);
    release(&ptable.lock);
    info("user_init ok");
}

/*
 * Per-CPU process scheduler.
 * Each CPU calls scheduler() after setting itself up.
 * Scheduler never returns. It loops, doing:
 * - choose a process to run
 * - swtch to start running that process
 * - eventually that process transfers control
 *   via swtch back to the scheduler.
 */
void
scheduler(void)
{
    idle_init();
    for (struct proc * p;;) {
        acquire(&ptable.lock);
        struct list_head *head = &ptable.sched_que;
        if (list_empty(head)) {
            p = thiscpu()->idle;
        } else {
            p = container_of(list_front(head), struct proc, link);
            list_pop_front(head);
        }
        uvm_switch(p->pgdir);
        thiscpu()->proc = p;
        swtch(&thiscpu()->scheduler, p->context);
        release(&ptable.lock);
    }
}


/*
 * A fork child's very first scheduling by scheduler()
 * will swtch here. "Return" to user space.
 */
static void recycle_proc(void *arg);

static void
forkret(void)
{
    #include <fs/v6/file.h>

    extern device_t root_dev;
    extern struct mount_ops procfs_mount_ops;
    extern struct mount_ops fat_mount_ops;
    int err;

    static int first = 1;
    if (first && thisproc() != thiscpu()->idle) {
        first = 0;
        release(&ptable.lock);

        //v6_init();
        sd_postinit();      // パーティション情報のセット
        v6_set_super();     // v6スーパーブロックの読み込みとmp->superのセット
        err = vfs_mount(NULL, "/proc", DEVPROCFS, &procfs_mount_ops, VFS_MBF_READ_ONLY, 0);
        if (err) {
            error("mount /proc is failed: %d", err);
        }
#ifdef CONFIG_FAT
        err = vfs_mount(NULL, "/d/", DEVFAT2, &fat_mount_ops, 0, 0);
        if (err) {
            error("mount /d/ is failed: %d", err);
        }
#endif

#if 1
        usb_init();
        net_init();
        net_run();
        kthread_create("ether", kthread_read_ether, NULL);
        workqueue_init();
        //queue_work(&wq, recycle_proc, 0);
#endif
    } else {
        release(&ptable.lock);
    }
    trace("forkret: proc '%s'(%d)", thisproc()->name, thisproc()->pid);
}

/* Give up CPU. */
void
yield(void)
{
    struct proc *p = thisproc();
    acquire(&ptable.lock);
    if (p != thiscpu()->idle)
        list_push_back(&ptable.sched_que, &p->link);
    p->state = RUNNABLE;
    swtch(&p->context, thiscpu()->scheduler);
    p->state = RUNNING;
    release(&ptable.lock);
}

/*
 * Atomically release lock and sleep on chan.
 * Reacquires lock when awakened.
 */
void
sleep(void *chan, struct spinlock *lk)
{
    struct proc *p = thisproc();
    int i = HASH(chan);
    if (p == thiscpu()->idle) {
        info("[%d] pid=%c, chan=0x%p", cpuid(), p->pid, chan);
    }
    assert(i < SQSIZE);
    assert(p != thiscpu()->idle);

    if (lk != &ptable.lock) {
        acquire(&ptable.lock);
        release(lk);
    }

    p->chan = chan;
    list_push_back(&ptable.slpque[i], &p->link);

    p->state = SLEEPING;
    trace("'%s'(%d) sleep lk=0x%p", p->name, p->pid, lk);
    swtch(&thisproc()->context, thiscpu()->scheduler);
    trace("'%s'(%d) wakeup lk=0x%p", p->name, p->pid, lk);
    p->state = RUNNING;

    if (lk != &ptable.lock) {
        release(&ptable.lock);
        acquire(lk);
    }
}

/*
 * Wake up all processes sleeping on chan.
 * The ptable lock must be held.
 */
static void
wakeup1(void *chan)
{
    struct list_head *q = &ptable.slpque[HASH(chan)];
    struct proc *p, *np;

    LIST_FOREACH_ENTRY_SAFE(p, np, q, link) {
        if (p->chan == chan) {
            trace("wake '%s'(%d)", p->name, p->pid);
            list_drop(&p->link);
            list_push_back(&ptable.sched_que, &p->link);
            p->state = RUNNABLE;
        }
    }
}

/* Wake up all processes sleeping on chan. */
void
wakeup(void *chan)
{
    acquire(&ptable.lock);
    wakeup1(chan);
    release(&ptable.lock);
}

/*
 * Create a new process copying p as the parent.
 * Sets up stack to return as if from system call.
 * Caller must set state of returned proc to RUNNABLE.
 */
int
fork(void)
{
    int ret = 0;
    struct proc *cp = thisproc();
    trace("call proc_alloc for fork");
    struct proc *np = proc_alloc();

    if (np == 0) {
        error("proc_alloc returns null");
        return -1;
    }

    // 親プロセスから子プロセスにvmasをコピーする
    trace("call copy_vmas: p: %d, np: %d", cp->pid, np->pid);
    if ((ret = copy_vmas(cp, np)) < 0) {
        //trace("ret=%d", ret);
        acquire(&ptable.lock);
        np->state = UNUSED;
        release(&ptable.lock);
        error("failed copy_vmas");
        return ret;
    } else {
        //print_mmap_list(cp, "fork cp");
        //print_mmap_list(np, "fork np");
    }
    trace("call uvm_copy: pgdir: 0x%llx", cp->pgdir);
    if ((np->pgdir = uvm_copy(cp->pgdir)) == 0) {
        kfree(np->kstack);

        acquire(&ptable.lock);
        np->state = UNUSED;
        release(&ptable.lock);

        error("uvm_copy returns null");
        return -1;
    }

    np->parent = cp;

    np->base = cp->base;
    np->sz = cp->sz;
    np->stksz = cp->stksz;

    memmove(np->tf, cp->tf, sizeof(*np->tf));

    // Fork returns 0 in the child.
    np->tf->x[0] = 0;

    dup_fd_table(np->fd_table, cp->fd_table);
    np->fdflag = cp->fdflag;

    np->cwd = vfs_clone_vnode(cp->cwd);
    np->pgid = cp->pgid;
    np->sid = cp->sid;
    np->uid = cp->uid;
    np->gid = cp->gid;
    np->egid = cp->egid;
    np->sgid = cp->sgid;
    np->fsgid = cp->fsgid;
    np->ngroups = cp->ngroups;
    memmove(np->groups, cp->groups, sizeof(gid_t) * cp->ngroups);
    np->cap_effective = cp->cap_effective;
    np->cap_inheritable = cp->cap_inheritable;
    np->cap_permitted = cp->cap_permitted;

    memmove(&np->signal, &cp->signal, sizeof(struct signal));
    np->signal.pending = 0;

    np->stime = np->utime = 0;
    np->it_real_value = np->it_real_incr = 0;
    init_timer(&np->real_timer);
    np->real_timer.data = (uint64_t) np;
    np->real_timer.fn = it_real_fn;

    int pid = np->pid;

    acquire(&ptable.lock);
    list_push_back(&cp->child, &np->clink);
    list_push_back(&ptable.sched_que, &np->link);
    np->state = RUNNABLE;
    release(&ptable.lock);

    trace("'%s'(%d) fork '%s'(%d)", cp->name, cp->pid, np->name, np->pid);

    return pid;
}


/*
 * Wait for a child process to exit and return its pid.
 * Return -1 if this process has no children.
 */
#if 0
int wait(void)
{
    struct proc *cp = thisproc();

    struct list_head *q = &cp->child;
    struct proc *p, *np;

    acquire(&ptable.lock);
    while (!list_empty(q)) {
        LIST_FOREACH_ENTRY_SAFE(p, np, q, clink) {
            if (p->state == ZOMBIE) {
                assert(p->parent == cp);

                list_drop(&p->clink);

                kfree(p->kstack);
                vm_free(p->pgdir);
                p->state = UNUSED;

                int pid = p->pid;
                release(&ptable.lock);
                return pid;
            }
        }
        //info("[%d] pid=%c, chan=0x%p", cpuid(), thisproc()->pid, cp);
        sleep(cp, &ptable.lock);
    }
    release(&ptable.lock);
    return -1;
}
#endif

/*
 * Wait for a child process to exit and return its pid.
 * Return -1 if this process has no children.
 */
int
wait4(pid_t pid, int *status, int options, struct rusage *ru)
{
    struct proc *cp = thisproc();
    struct list_head *que = &cp->child;
    struct proc *p, *np;

    acquire(&ptable.lock);
    while (!list_empty(que)) {
        LIST_FOREACH_ENTRY_SAFE(p, np, que, clink) {
            if (p->parent != cp) continue;
            if (pid > 0) {
                if (p->pid != pid)
                    continue;
            } else if (pid == 0) {
                if (p->pgid != cp->pgid)
                    continue;
            } else if (pid != -1) {
                if (p->pgid != -pid)
                    continue;
            }
            if (p->state == ZOMBIE
             || (options & WUNTRACED && p->state == SLEEPING)
             || (options & WNOHANG)) {
                //assert(p->parent == cp);

                if (status) *status = p->xstate << 8;
                if (ru) memset(ru, 0, sizeof(struct rusage));

                list_drop(&p->clink);

                kfree(p->kstack);
                vm_free(p->pgdir);
                p->state = UNUSED;

                int pid = p->pid;
                release(&ptable.lock);
                return pid;
            }
        }
        sleep(cp, &ptable.lock);
    }
    release(&ptable.lock);
    return -ECHILD;
}


/*
 * Exit the current process.  Does not return.
 * An exited process remains in the zombie state
 * until its parent calls wait() to find out it exited.
 */
void
exit(int err)
{
    struct proc *cp = thisproc();
    if (cp == initproc)
        panic("init exit");

    if (err) {
        trace("exit: pid %d, err %d", cp->pid, err);
    }

    // vmaを解除する
    //print_mmap_list(p, "before exit");
    if (cp->vmas) {
        struct vma *vma = cp->vmas;
        while (vma) {
            if (vma->f)
                trace("pid[%d] f->ip: %d refcnt[1]: %d", cp->pid, vma->f->vnode->ino, vma->f->vnode->refcount);
            munmap(vma->addr, vma->length);
            vma = vma->next;
        }
        cp->vmas = NULL;
    }
    //print_mmap_list(p, "after  exit");

    // Close all open files.
    release_fd_table(cp->fd_table);

    trace("ino: %d, refcount = %d", cp->cwd->ino, cp->cwd->refcount);
    vfs_release_vnode(cp->cwd);
    cp->cwd = 0;

    // タイマーを削除
    del_timer_sync(&cp->real_timer, false);

    acquire(&ptable.lock);

    // Parent might be sleeping in wait().
    wakeup1(cp->parent);

    // Pass abandoned children to init.
    struct list_head *q = &cp->child;
    struct proc *p, *np;
    LIST_FOREACH_ENTRY_SAFE(p, np, q, clink) {
        assert(p->parent == cp);
        p->parent = initproc;
        list_drop(&p->clink);
        list_push_back(&initproc->child, &p->clink);
        if (p->state == ZOMBIE) {
            wakeup1(initproc);
        }
    }
    assert(list_empty(q));
    // Jump into the scheduler, never to return.
    cp->xstate = err & 0x7f;
    cp->state = ZOMBIE;

    swtch(&cp->context, thiscpu()->scheduler);
    release(&ptable.lock);
    panic("zombie exit");
}

/*
 * Print a process listing to console. For debugging.
 * Runs when user types ^P on console.
 */
void
procdump(void)
{
    static char *states[] = {
        [UNUSED]   "unused  ",
        [EMBRYO]   "embryo  ",
        [SLEEPING] "sleeping",
        [RUNNABLE] "runnable",
        [RUNNING]  "running ",
        [ZOMBIE]   "zombie  "
    };
    struct proc *p;

    // Donot acquire ptable.lock to avoid deadlock
    // acquire(&ptable.lock);
    cprintf("\n");
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state == UNUSED)
            continue;
        if (p->parent)
            cprintf("%-3d %s %s ppid: %d\n", p->pid, states[p->state], p->name,
                    p->parent->pid);
        else
            cprintf("%-3d %s %s\n", p->pid, states[p->state], p->name);
    }
    // release(&ptable.lock);
}

extern uint64_t kpgdir;

static void kthread_exit(void)
{
    struct proc *p = thisproc();

    if (!p->iskthread)
        panic("kthread_exit: not a kthread");

    acquire(&ptable.lock);
    wakeup(p);
    p->state = ZOMBIE;
    list_drop(&p->link);
    release(&ptable.lock);

    yield();

}

/* ワークキューの常駐スレッド */
static void wq_worker(void *arg)
{
    struct workqueue *wq = (struct workqueue *)arg;

    for (;;) {
        acquire(&wq->lock);

        while (wq->head == 0 && !wq->shutdown) {
            sleep(wq, &wq->lock);
        }

        if (wq->shutdown && wq->head == 0) {
            release(&wq->lock);
            break;
        }

        struct work *w = wq->head;
        wq->head = w->next;
        if (wq->head == 0)
            wq->tail = 0;

        release(&wq->lock);

        if (w->func) {
            w->func(w->arg);
        }

        kmfree(w);
    }

    kthread_exit();
}

static void recycle_proc(void *arg)
{
    struct proc *p;

    acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->iskthread && p->state == ZOMBIE) {
            kfree(p->kstack);
            p->kstack = 0;
            p->state = UNUSED;
            p->iskthread = 0;
            p->fn_ptr = NULL;
            p->fn_arg = NULL;
        }
    }
    release(&ptable.lock);
}

static void kthread_stub(void)
{
    //release(&ptable.lock);

    struct proc *p = thisproc();
    assert(p);
    if (p->iskthread && p->fn_ptr) {
        p->fn_ptr(p->fn_arg);
    }

    kthread_exit();
}

void workqueue_init(void)
{
    initlock(&wq.lock, "workqueue");
    acquire(&wq.lock);
    wq.head = 0;
    wq.tail = 0;
    wq.shutdown = 0;

    wq.worker = kthread_create("workqueue", wq_worker, &wq);
    release(&wq.lock);
    info("workqueue_init ok, worker pid - %d", wq.worker->pid);
}

int queue_work(struct workqueue *wq, void (*func)(void *), void *arg)
{
    struct work *w = kmalloc(sizeof(struct work));
    if (w == 0) {
        error("no memory");
        return -ENOMEM;
    }

    w->func = func;
    w->arg = arg;
    w->next = 0;

    acquire(&wq->lock);
    if (wq->tail == 0) {
        wq->head = w;
        wq->tail = w;
    } else {
        wq->tail->next = w;
        wq->tail = w;
    }

    wakeup(wq);

    release(&wq->lock);

    return 0;
}

struct proc *kthread_create(const char *name, void(*func)(void *), void *param)
{
    struct proc *p;

    p = proc_alloc();
    p->pgdir = (void *)kpgdir;

    p->sz = PGSIZE;
    safestrcpy(p->name, name, sizeof(p->name));

    p->iskthread = 1;
    p->fn_ptr = func;
    p->fn_arg = param;
    p->context->lr = (uint64_t)kthread_stub;

    p->cwd = 0;
    p->state = RUNNABLE;
    acquire(&ptable.lock);
    list_push_back(&ptable.sched_que, &p->link);
    release(&ptable.lock);

    return p;
}

void kthread_read_ether(void *param)
{
    while(1) {
#ifdef USING_RASPI
        lan7800_net_handler();
#else
        usb_cdcether_net_handler();
#endif
        //thisproc()->state = RUNNABLE;
        yield();
    }
}

struct proc *get_proc(pid_t pid)
{
    struct proc *p;

    acquire(&ptable.lock);
    for (p = ptable.proc; p < ptable.proc + NPROC; p++) {
        if (p->pid == pid) {
            release(&ptable.lock);
            return p;
        }
    }
    release(&ptable.lock);
    return NULL;
}

void proc_iter_start(struct process_iter *iter)
{
    iter->slot = 0;
}

struct proc *proc_iter_next(struct process_iter *iter)
{
    struct proc *proc;

    do {
        if (iter->slot >= NPROC)
            return NULL;
        proc = &ptable.proc[iter->slot++];
    } while (proc->pid == 0);

    return proc;
}

// シグナルハンドラ関数

// プロセスを停止する
void term_handler(struct proc *p)
{
    trace("pid: %d", p->pid);
    acquire(&ptable.lock);
    p->killed = 1;
    if (p->state == SLEEPING)
        p->state = RUNNABLE;
    release(&ptable.lock);
}

// プロセスを継続する
void cont_handler(struct proc *p)
{
    wakeup1(p);
}

// プロセスを停止する
void stop_handler(struct proc *p)
{
    acquire(&q.lock);
    sleep(p, &q.lock);
    release(&q.lock);
}

// ユーザハンドラを処理する
void user_handler(struct proc *p, int sig)
{
    extern void dump_tf(struct trapframe *tf);
    acquire(&q.lock);
    trace("sig=%d", sig);
    uint64_t sp = p->tf->sp;
    trace("sp1: 0x%llx", sp);
    // 現在のトラップフレームをユーザスタックに保存する
    sp -= sizeof(struct trapframe);
    sp = ROUNDDOWN(sp, 0x10);
    memmove((void *)sp, (void *)p->tf, sizeof(struct trapframe));
    p->oldtf = (struct trapframe *)sp;
    trace("sp2: 0x%llx", sp);

    // sigret_syscall.Sのコードをユーザスタックにプッシュする
    // a) コードの開始アドレスをサイズを求める
    void *sig_ret_code_addr = (void *)execute_sigret_syscall_start;
    uint64_t sig_ret_code_size = (uint64_t)&execute_sigret_syscall_end - (uint64_t)&execute_sigret_syscall_start;

    // b) コードをスタックにコピーし、その先頭アドレスを変数handler_ret_addにセットする
    sp -= sig_ret_code_size;
    sp = ROUNDDOWN(sp, 0x10);
    uint64_t handler_ret_addr = sp;
    memmove((void *)sp, sig_ret_code_addr, sig_ret_code_size);
    trace("sig_ret: start: 0x%llx, size: 0x%llx", sig_ret_code_addr, sig_ret_code_size);
    trace("sp3: 0x%llx", sp);

    // コードの引数のシグナル番号をセットする
    p->tf->x[0] = sig;

    // コードのリターンアドレスをスタックにプッシュする
    sp -= sizeof(uint64_t);
    sp = ROUNDDOWN(sp, 0x10);
    memmove((void *)sp, (void *)&handler_ret_addr, sizeof(uint64_t));
    trace("sp4: 0x%llx", sp);

    // セットしたspをトラップフレームのspにセットする
    p->tf->sp = sp;

    // ユーザハンドラを実行するようにeipを変更する
    p->tf->elr = (uint64_t)p->signal.actions[sig].sa_handler;
    //dump_tf(p->tf);
    release(&q.lock);
}

// シグナルを処理する
void handle_signal(struct proc *p, int sig)
{
    trace("[%d]: sig=%d, handler=0x%llx", p->pid, sig, p->signal.actions[sig].sa_handler);
    if (!sig) return;
    if (p->signal.actions[sig].sa_handler == SIG_IGN) {
        trace("sig %d handler is SIG_IGN", sig);
    } else if (p->signal.actions[sig].sa_handler == SIG_DFL) {
        switch(sig) {
            case SIGSTOP:
            case SIGTSTP:
            case SIGTTIN:
            case SIGTTOU:
                stop_handler(p);
                break;
            case SIGCONT:
                cont_handler(p);
                break;
            case SIGABRT:
            case SIGBUS:
            case SIGFPE:
            case SIGILL:
            case SIGQUIT:
            case SIGSEGV:
            case SIGSYS:
            case SIGTRAP:
            case SIGXCPU:
            case SIGXFSZ:
                // Core: through
            case SIGALRM:
            case SIGHUP:
            case SIGINT:
            case SIGIO:
            case SIGKILL:
            case SIGPIPE:
            case SIGPROF:
            case SIGPWR:
            case SIGTERM:
            case SIGSTKFLT:
            case SIGUSR1:
            case SIGUSR2:
            case SIGVTALRM:
                term_handler(p);
                break;
            case SIGCHLD:
            case SIGURG:
            case SIGWINCH:
                // Doubt - ignore handler()
                break;
            default:
                break;
        }
    } else {
        trace("call user_handler: sig = %d", sig);
        user_handler(p, sig);
    }

    // 保留シグナルフラグをクリアする
    acquire(&q.siglock);
    sigdelset(&p->signal.pending, sig);
    release(&q.siglock);
}

// trap処理終了後、ユーザモードに戻る前に実行される
void check_pending_signal(void)
{
    struct proc *p = thisproc();

    for (int sig = 0; sig < NSIG; sig++) {
        if (sigismember(&p->signal.pending, sig) == 1) {
            trace("pid=%d, sig=%d", p->pid, sig);
            handle_signal(p, sig);
            break;
        }
    }
}

// 親から引き継いだsignalを調整する
void flush_signal_handlers(struct proc *p)
{
    struct sigaction *ka;

    for (int i = 0; i < NSIG; i++) {
        ka = &p->signal.actions[i];
        if (ka->sa_handler != SIG_IGN)
            ka->sa_handler = SIG_DFL;
        ka->sa_flags = 0;
        sigemptyset(&ka->sa_mask);
    }
    p->paused = 0;
}

// IDがpidのプロセスにシグナルsigを送信する
static void send_signal(struct proc *p, int sig)
{
    trace("pid=%d, sig=%d, state=%d, paused=%d", p->pid, sig, p->state, p->paused);
    if (sig == SIGKILL) {
        p->killed = 1;
    } else {
        if (!sigismember(&p->signal.pending, sig)) {
            sigaddset(&p->signal.pending, sig);
            trace("set sig %d to pending 0x%llx", sig, p->signal.pending);
        } else {
            trace("sig %d is already pending", sig);
        }
    }

    if (p->state == SLEEPING) {
        if (p->paused == 1 && (sig == SIGTERM || sig == SIGINT || sig == SIGKILL)) {
            // pause()でSLEEPINGのプロセス
            p->paused = 0;
            handle_signal(p, SIGCONT);
        } else if (p->paused == 0 && p->killed != 1) {
            // 停止中のプロセス
            handle_signal(p, sig);
        }
    }
}

// sys_kill()の処理関数
long kill(pid_t pid, int sig)
{
    struct proc *current = thisproc();
    struct proc *p;
    long error = -ESRCH;
    trace("pid: %d, sig: %d", pid, sig);
    if (pid == 0 || pid < -1) {
        pid_t pgid = pid == 0 ? current->pgid : -pid;
        if (pgid > 0) {
            error = -ESRCH;
            acquire(&ptable.lock);
            for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
                if (p->pgid == pgid) {
                    send_signal(p, sig);
                    error = 0;
                }
            }
            release(&ptable.lock);
        }
        return error;
    } else if (pid == -1) {
        acquire(&ptable.lock);
        for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
            if (p->pid > 1 && p != current) {
                send_signal(p, sig);
                error = 0;
            }
        }
        release(&ptable.lock);
        return error;
    } else {
        acquire(&ptable.lock);
        for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
            if (p->pid == pid) {
                send_signal(p, sig);
                error = 0;
            }
        }
        release(&ptable.lock);
        return error;
    }
    return -EINVAL;
}

// sys_rt_sigsuspend()の処理関数
long sigsuspend(sigset_t *mask)
{
    struct proc *p = thisproc();
    sigset_t oldmask;

    acquire(&q.siglock);
    p->paused = 1;
    sigdelset(mask, SIGKILL);
    sigdelset(mask, SIGSTOP);
    oldmask = p->signal.mask;
    siginitset(&p->signal.mask, mask);
    release(&q.siglock);

    acquire(&q.lock);
    sleep(p, &q.lock);
    release(&q.lock);

    acquire(&q.siglock);
    p->signal.mask = oldmask;
    release(&q.siglock);
    return -EINTR;
}

// sys_rt_sigaction()の処理関数
long sigaction(int sig, struct k_sigaction *act,  struct k_sigaction *oldact)
{
    acquire(&q.siglock);
    struct signal *signal = &thisproc()->signal;
    if (oldact) {
        struct sigaction *action = &signal->actions[sig];
        oldact->handler = action->sa_handler;
        oldact->flags = (unsigned long)action->sa_flags;
        oldact->restorer = action->sa_restorer;
        memmove((void *)&oldact->mask, &action->sa_mask, 8);
        trace("oldact=0x%llx", oldact->handler);
    }
    if (act) {
        struct sigaction *action = &signal->actions[sig];
        action->sa_handler = act->handler;
        action->sa_flags = (int)act->flags;
        action->sa_restorer = act->restorer;
        memmove((void *)&action->sa_mask, &act->mask, 8);
        signal->mask = action->sa_mask;
        sigdelset(&signal->mask, SIGKILL);
        sigdelset(&signal->mask, SIGSTOP);
        trace("sig=%d handler: act=0x%llx, p=0x%llx", sig, act->handler, action->sa_handler);
    }
    release(&q.siglock);

    return 0;
}

// sys_rt_sigpending()の処理関数
long sigpending(sigset_t *pending)
{
    struct proc *p = thisproc();

    acquire(&q.siglock);
    *pending = p->signal.pending;
    release(&q.siglock);
    return 0;
}

// sys_rt_sigprocmask()の処理関数
long sigprocmask(int how, sigset_t *set, sigset_t *oldset, size_t size)
{
    int ret = 0;

    acquire(&q.siglock);
    struct signal *signal = &thisproc()->signal;
    trace("how=%d oldmask=0x%llx set=0x%llx, size=%lld", how, oldmask, set, size);
    if (oldset)
        *oldset = signal->mask;
    if (set) {
        switch(how) {
            case SIG_BLOCK:
                sigorset(&signal->mask, &signal->mask, set);
                break;
            case SIG_UNBLOCK:
                signotset(set, set);
                sigandset(&signal->mask, &signal->mask, set);
                break;
            case SIG_SETMASK:
                siginitset(&signal->mask, set);
                //signal->mask = *set;
                break;
            default:
                ret = -EINVAL;
        }
    }
    trace(" newmask=0x%llx", signal->mask);
    release(&q.siglock);
    return ret;
}

// sys_rt_sigreturn()の処理関数
long sigreturn(void)
{
    struct proc *p = thisproc();

    memmove((void *)p->tf, (void *)p->oldtf, sizeof(struct trapframe));
    return 0;
}

// sys_ppoll()の処理関数
long ppoll(struct pollfd *fds, nfds_t nfds, struct timespec *timeout_ts, sigset_t *sigmask)
{
    struct proc *p = thisproc();
    sigset_t old_sigmask;
    long timeout;

    // TODO: timeout処理
#if 0
    timeout = (timeout_ts == NULL) ? -1 :
        timeout_ts->tv_sec * 1000000 + (timeout_ts->tv_nsec + 999) / 1000;

    if (sigmask)
        sigprocmask(SIG_SETMASK, sigmask, &old_sigmask, sizeof(sigset_t));
#endif

    if (fds == NULL) {
        trace("pid %d is paused", p->pid);
        p->paused = 1;
        acquire(&q.lock);
        sleep(p, &q.lock);
        release(&q.lock);
        trace("pid %d is woke up and return", p->pid);
        return -EINTR;
    }

    for (int i = 0; i < nfds; i++) {
        fds[i].revents = fds[i].fd == 0 ? POLLIN : POLLOUT;
    }

#if 0
    if (sigmask)
        sigprocmask(SIG_SETMASK, &old_sigmask, NULL, sizeof(sigset_t));
#endif

    return 0;
}

// sys_setpgid()の処理関数
long setpgid(pid_t pid, pid_t pgid)
{
    struct proc *current = thisproc(), *p, *pp;
    long error = -EINVAL;

    if (!pid) pid = current->pid;
    if (!pgid) pgid = pid;
    if (pgid < 0) return -EINVAL;

    if (pid != current->pid) {
        acquire(&ptable.lock);
        for (pp = ptable.proc; pp < &ptable.proc[NPROC]; pp++) {
            if (pp->pid == pid) {
                p = pp;
                break;
            }
        }
        release(&ptable.lock);
        error = -ESRCH;
        if (!p) goto out;
    } else {
        p = current;
    }

    error = -EINVAL;
    if (p->parent == current) {
        error = -EPERM;
        if (p->sid != current->sid) goto out;
    } else {
        error = -ESRCH;
        if (p != current) goto out;
    }

    if (pgid != pid) {
        acquire(&ptable.lock);
        for (pp = ptable.proc; pp < &ptable.proc[NPROC]; pp++) {
            if (pp->sid == current->sid) {
                release(&ptable.lock);
                goto ok_pgid;
            }
        }
        release(&ptable.lock);
        goto out;
    }

ok_pgid:
    if (current->pgid != pgid) {
        current->pgid = pgid;
    }
    error = 0;
out:
    return error;
}

// sys_getpgid()の処理関数
pid_t getpgid(pid_t pid)
{
    struct proc *p;

    if (!pid) {
        return thisproc()->pgid;
    } else {
        acquire(&ptable.lock);
        for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
            if (p->pid == pid) {
                release(&ptable.lock);
                return p->pgid;
            }
        }
        release(&ptable.lock);
        return -ESRCH;
    }
}
