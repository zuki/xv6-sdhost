#include <proc.h>

#include <string.h>
#include <types.h>
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
#include <linux/resources.h>
#include <linux/wait.h>
#include <linux/errno.h>

extern int sd_postinit(void);

extern void trapret();
extern void swtch(struct context **old, struct context *new);

static void forkret();
static void idle_init();

#define SQSIZE  0x100           /* Must be power of 2. */
#define HASH(x) ((((uint64_t)(x)) >> 5) & (SQSIZE - 1))

struct cpu cpu[NCPU];

struct {
    struct proc proc[NPROC];
    struct list_head slpque[SQSIZE];
    struct list_head sched_que;
    struct spinlock lock;
} ptable;

struct proc *initproc;
struct slab_cache *VMA;

static int pid = 0;

void
proc_init(void)
{
    initlock(&ptable.lock, "ptable");
    list_init(&ptable.sched_que);
    for (int i = 0; i < SQSIZE; i++)
        list_init(&ptable.slpque[i]);
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

    p->pid = ++pid;
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

    list_init(&p->child);

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

    p->tf->elr = 0;

    safestrcpy(p->name, name, sizeof(p->name));
    return p;
}

/* Initialize per-cpu idle process. */
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
static void
forkret(void)
{
    #include <fs/v6/file.h>

    extern device_t root_dev;
    extern struct mount_ops procfs_mount_ops;
    int err;

    static int first = 1;
    if (first && thisproc() != thiscpu()->idle) {
        first = 0;
        release(&ptable.lock);

        //v6_init();
        sd_postinit();
        v6_set_super();
        err = vfs_mount(NULL, "/proc", DEVPROCFS, &procfs_mount_ops, VFS_MBF_READ_ONLY, 0);
        if (err) {
            error("mount /proc is failed: %d", err);
        } else {
            trace("mount proc ok");
        }

#if 1
        usb_init();
        net_init();
        net_run();
        kthread_created(kthread_read_ether);
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
    struct proc *np = proc_alloc();

    if (np == 0) {
        error("proc_alloc returns null");
        return -1;
    }

    // 親プロセスから子プロセスにvmasをコピーする
    if ((ret = copy_vmas(cp, np)) < 0) {
        //debug("ret=%d", ret);
        acquire(&ptable.lock);
        np->state = UNUSED;
        release(&ptable.lock);
        error("failed copy_vmas");
        return ret;
    }

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
    np->cwd = vfs_clone_vnode(cp->cwd);
    np->pgid = cp->pgid;
    np->sid = cp->sid;
    np->uid = cp->uid;
    np->gid = cp->gid;
    np->fdflag = cp->fdflag;

    int pid = np->pid;

    acquire(&ptable.lock);
    list_push_back(&cp->child, &np->clink);
    list_push_back(&ptable.sched_que, &np->link);
    np->state = RUNNABLE;
    release(&ptable.lock);

    debug("'%s'(%d) fork '%s'(%d)", cp->name, cp->pid, np->name, np->pid);

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
        [UNUSED] "unused",
        [EMBRYO] "embryo",
        [SLEEPING] "sleep ",
        [RUNNABLE] "runble",
        [RUNNING] "run   ",
        [ZOMBIE] "zombie"
    };
    struct proc *p;

    // Donot acquire ptable.lock to avoid deadlock
    // acquire(&ptable.lock);
    for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
        if (p->state == UNUSED)
            continue;
        if (p->parent)
            cprintf("%d %s %s fa: %d\n", p->pid, states[p->state], p->name,
                    p->parent->pid);
        else
            cprintf("%d %s %s\n", p->pid, states[p->state], p->name);
    }
    // release(&ptable.lock);
}

extern uint64_t kpgdir;

void kthread_created(void(*func)(void))
{
    struct proc *p;

    p = proc_alloc();
    p->pgdir = (void *)kpgdir;

    // allocate one user page and copy init's instructions
    // and data into it.
    //uvminit(p->pagetable, initcode, sizeof(initcode));
    p->sz = PGSIZE;
    safestrcpy(p->name,"rxether",sizeof(p->name));
    p->context->lr = (uint64_t)func;

    // prepare for the very first "return" from kernel to user.
    //p->trapframe->epc = 0;      // user program counter
    //p->trapframe->sp = PGSIZE;  // user stack pointer

    //safestrcpy(p->name, "initcode", sizeof(p->name));
    p->cwd = 0;
    p->state = RUNNABLE;
    acquire(&ptable.lock);
    list_push_back(&ptable.sched_que, &p->link);
    release(&ptable.lock);
}

void kthread_read_ether(void)
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
