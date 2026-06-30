#ifndef INC_PROC_H
#define INC_PROC_H

#include <types.h>
#include <arm.h>
#include <mmu.h>
#include <trap.h>
#include <spinlock.h>
#include <list.h>
#include <filedesc.h>
#include <slab.h>
#include <linux/resources.h>
#include <linux/signal.h>
#include <linux/ppoll.h>
#include <linux/time.h>
#include <linux/capability.h>

#define NPROC           100     /* 最大プロセス数 */
#define NCPU            4       /* コア数 */
#define NGROUPS         32      /* ユーザが所属できる最大グループ数 */

extern struct slab_cache *VMA;

/* Stack must always be 16 bytes aligned. */
struct context {
    uint64_t lr0, lr, fp;
    uint64_t x[10];             /* X28 ... X19 */
    uint64_t padding;
    // uint64_t q0[2];             /* V0 */
};

/* vma (virtual memory map area)構造体 */
struct vma {
    void *          addr;       /* 先頭アドレス */
    size_t          length;     /* 領域長 */
    int             prot;       /* vma属性 */
    int             flags;      /* vmaフラグ */
    struct vfile *  f;          /* mapしているファイル */
    off_t           offset;     /* この領域のファイル内のオフセット */
    struct vma *    next;       /* 次のvmaへのポインタ */
};

struct signal {
    sigset_t            mask;
    sigset_t            pending;
    struct sigaction    actions[NSIG];
};

enum procstate { UNUSED, EMBRYO, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };


/* Per-process state */
struct proc {
    /*
     * Memory layout
     *
     * +----------+
     * |  Kernel  |
     * +----------+  KERNBASE
     * |  Stack   |
     * +----------+  KERNBASE - stksz
     * |   ....   |
     * |   ....   |
     * +----------+  base + sz
     * |   Heap   |
     * +----------+
     * |   Code   |
     * +----------+  base
     * | Reserved |
     * +----------+  0
     *
     */
    size_t base;                /* プロセスのベースアドレス */
    size_t sz;                  /* プロセスメモリサイズ */
    size_t stksz;               /* プロセスのスタックサイズ */

    void *pgdir;                /* ユーザ空間のページテーブル */
    void *kstack;               /* カーネルスタック￥ */
    enum procstate state;       /* プロセスの状態 */
    int pid;                    /* プロセスID. */
    pid_t pgid;                 // プロセスグループID
    pid_t sid;                  // セッションID
    struct proc *parent;        /* 親プロセス */
    struct list_head child;     /* このプロセスの子供リスト */
    struct list_head clink;     /* 親の子供リストへのリンク用 */

    uid_t uid, euid, suid, fsuid;   /* ユーザID */
    gid_t gid, egid, sgid, fsgid;   /* グループID */
    gid_t groups[NGROUPS];      /* 所属グループ */
    int   ngroups;              /* 実際に所属しているグループ数 */

    kernel_cap_t   cap_effective, cap_inheritable, cap_permitted;   /* 権限 */

    mode_t  umask;              /* umask */

    struct trapframe *tf;       /* カレントシステムコールのトラップフレーム */
    struct context *context;    /* コンテキスト: swtch() here to run process. */
    struct list_head link;      /* sched_que/sleepqueへのリンク用 */

    void *chan;                 /* スリープチャンネル */
    int killed;                 /* killされたか否か */
    int xstate;                 // waitで待っていてる親に返すexit status

    struct spinlock time_lock;      /* timer関連のフィールドを保護 */
    uint64_t stime, utime;          /* ticks for system and user */
    uint64_t it_real_value;         /* interval timer interval値 */
    uint64_t it_real_incr;          /* interval timer increment値 */
    struct timer_list real_timer;   /* Real timerリスト */

    int fdflag;                 /* ファイルディスクリプタフラグ */
    fd_table_t fd_table;        /* オープンファイル管理 */

    struct vnode *cwd;          /* カレントディレクトリ */
    char name[16];              /* プロセス名（デバッグ用） */

    struct vma  *vmas;          /* vma領域のリストの先頭のポインタ */

    struct signal signal;       /* シグナルを保持 */
    struct trapframe *oldtf;    /* トラップフレームを保存 */
    int paused;                 /* 停止中か? */
    void *userdata;             /* proc/thread固有データへのポインタ */
};

/* Per-CPU state */
struct cpu {
    struct context *scheduler;  /* swtch() here to enter scheduler */
    struct proc *proc;          /* The process running on this cpu or null. */
    struct proc *idle;          /* The idle process. */
    volatile int started;       /* Has the CPU started? */
    struct spinlock lock;
};

struct process_iter {
    int slot;
};

struct procfs_position {
    uint16_t slot;
    struct process_iter iter;
};

extern struct cpu cpu[NCPU];

static inline struct cpu *
thiscpu()
{
    return &cpu[cpuid()];
}

static inline struct proc *
thisproc()
{
    return thiscpu()->proc;
}

void proc_init(void);
void user_init(void);
void scheduler(void);
void sleep(void *chan, struct spinlock *lk);
void wakeup(void *chan);
void yield(void);
void exit(int err);
int  wait4(pid_t pid, int *status, int options, struct rusage *ru);
int  fork(void);
void procdump();
void kthread_read_ether(void *);
void kthread_created(const char *name, void(*func)(void *), void *param);
struct proc *get_proc(pid_t pid);
void proc_iter_start(struct process_iter *iter);
struct proc *proc_iter_next(struct process_iter *iter);

long kill(pid_t pid, int sig);
long sigsuspend(sigset_t *mask);
long sigaction(int sig, struct k_sigaction *act, struct k_sigaction *oldact);
long sigpending(sigset_t *pending);
long sigprocmask(int how, sigset_t *set, sigset_t *oldset, size_t size);
long sigreturn(void);
void check_pending_signal(void);
void stop_handler(struct proc *p);
void cont_handler(struct proc *p);
void term_handler(struct proc *p);
void handle_signal(struct proc *p , int sig);
void user_handler(struct proc *p, int sig);
void flush_signal_handlers(struct proc *p);

long ppoll(struct pollfd *fds, nfds_t nfds, struct timespec *timeout_ts, sigset_t *sigmask);
long setpgid(pid_t, pid_t);
pid_t getpgid(pid_t);
uint16_t get_procs();

// sigret_syscall.S
void execute_sigret_syscall_start(void);
void execute_sigret_syscall_end(void);

static inline int capable(int cap)
{
    if (cap_raised(thisproc()->cap_effective, cap))
        return 1;
    return 0;
}

#endif
