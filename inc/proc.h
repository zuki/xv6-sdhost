#ifndef INC_PROC_H
#define INC_PROC_H

#include <types.h>
#include <arm.h>
#include <mmu.h>
#include <trap.h>
#include <spinlock.h>
#include <list.h>
#include <filedesc.h>

#define NPROC           100     /* 最大プロセス数 */
#define NCPU            4       /* コア数 */
#define NGROUPS         32      /* ユーザが所属できる最大グループ数 */

/* Stack must always be 16 bytes aligned. */
struct context {
    uint64_t lr0, lr, fp;
    uint64_t x[10];             /* X28 ... X19 */
    uint64_t padding;
    // uint64_t q0[2];             /* V0 */
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
    size_t base, sz;
    size_t stksz;

    void *pgdir;                /* ユーザ空間のページテーブル */
    void *kstack;               /* カーネルスタック￥ */
    enum procstate state;       /* プロセスの状態 */
    int pid;                    /* プロセスID. */
    struct proc *parent;        /* 親プロセス */
    struct list_head child;     /* このプロセスの子供リスト */
    struct list_head clink;     /* 親の子供リストへのリンク用 */

    struct trapframe *tf;       /* カレントシステムコールのトラップフレーム */
    struct context *context;    /* コンテキスト: swtch() here to run process. */
    struct list_head link;      /* 実行プロセスリストへのリンク用 */
    void *chan;                 /* スリープチャンネル */
    uid_t   uid;
    gid_t   gid;
    mode_t  umask;
#if 0
    uid_t uid, euid, suid, fsuid;   /* ユーザID */
    gid_t gid, egid, sgid, fsgid;   /* グループID */
    gid_t groups[NGROUPS];      /* 所属グループ */
#endif
    int killed;                 /* killされたか否か */
    int fdflag;                 /* ファイルディスクリプタフラグ */
    fd_table_t fd_table;        /* オープンファイル管理 */
    struct vnode *cwd;          /* カレントディレクトリ */
    char name[16];              /* プロセス名（デバッグ用） */
};

/* Per-CPU state */
struct cpu {
    struct context *scheduler;  /* swtch() here to enter scheduler */
    struct proc *proc;          /* The process running on this cpu or null. */
    struct proc *idle;          /* The idle process. */
    volatile int started;       /* Has the CPU started? */
    struct spinlock lock;
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
int  wait(void);
int  fork(void);
void procdump();
void kthread_read_ether(void);
void kthread_created(void(*func)(void));

#endif
