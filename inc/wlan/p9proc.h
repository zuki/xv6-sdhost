#ifndef H_INC_WLAN_P9PROC_H
#define H_INC_WLAN_P9PROC_H

#include <types.h>
#include <wlan/p9util.h>
#include <wlan/p9error.h>
#include <spinlock.h>

// マルチCPUの競争を防ぐためのロック
typedef struct lock_t
{
    uint32_t _lock;
} Lock;

#if 0
#define lock        __p9lock
#define unlock      __p9unlock
void lock (Lock *l);
void unlock (Lock *l);
#endif

// ロックを得るまで実行権を明け渡して待機するロック
typedef struct qlock_t
{
    volatile int locked;
} QLock;

void qlock(QLock *qlock);
void qunlock(QLock *qlock);
int canqlock(QLock *qlock);

typedef struct Rendez
{
    // for lock chan
} Rendez;

typedef int sleephandler_t(void *param);
void p9sleep(Rendez *rendez, sleephandler_t *handler, void *param);
void p9tsleep(Rendez *rendez, sleephandler_t *handler, void *param, unsigned msecs);
void p9wakeup(Rendez *rendez);
int return0(void *param);

//#define p9kproc           __p9kproc
//void kproc(const char *name, void (*func) (void *param), void *param);

#define up              __p9up
extern struct up_t
{
    Rendez sleep;
    const char *errstr;
    char genbuf[1000];
    struct error_stack_t errstack;
} *up;

void p9proc_init(void);

#endif
