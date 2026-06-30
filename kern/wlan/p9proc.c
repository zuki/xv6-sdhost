#include <types.h>
#include <wlan/p9proc.h>
#include <wlan/p9arch.h>
#include <spinlock.h>
#include <proc.h>

#define SPINLOCK_SAVE_POWER

extern long get_ticks(void);
extern void yield(void);        // proc.c

#if 0
// ロックを取得する
void lock(Lock *l)
{
    EnterCritical (IRQ_LEVEL);

    // AARCH == 64 : Acquiring a lock with Load-Acquire Exclusive, Store-Release Exclusive
    // See: ARMv8-A Architecture Reference Manual, Section K10.3.1
    asm volatile
    (
        "mov x1, %0\n"
        "mov w2, #1\n"
        "prfm pstl1keep, [x1]\n"
        "1: ldaxr w3, [x1]\n"
        "cbnz w3, 1b\n"
        "stxr w3, w2, [x1]\n"
        "cbnz w3, 1b\n"

        : : "r" ((uintptr_t) &l->_lock) : "x1", "x2", "x3"
    );
}

// ロックを開放する
void unlock (Lock *l)
{
    // See: ARMv8-A Architecture Reference Manual, Section K10.3.2
    asm volatile
    (
        "mov x1, %0\n"
        "stlr wzr, [x1]\n"

        : : "r" ((uintptr_t) &l->_lock) : "x1"
    );

    LeaveCritical ();
}
#endif

// qlockを取得する（取得できない間は実行権を明け渡す）
void qlock (QLock *qlock)
{
    do {
        yield();
    } while (qlock->locked);

    qlock->locked = 1;
}

// qlockを開放する（開放したあと実行権を明け渡す）
void qunlock (QLock *qlock)
{
    assert (qlock->locked);
    qlock->locked = 0;
    yield();
}


// qlockが取得できれば取得して1を返す。取得できなければ実行権を明け渡して0を返す
int canqlock (QLock *qlock)
{
    if (qlock->locked) {
        yield();
        return 0;
    }
    qlock->locked = 1;
    return 1;
}

// スリープする（スリープハンドラを実行する）
void p9sleep(Rendez *rendez, struct spinlock *lk, sleephandler_t *handler, void *param)
{
    acquire(lk);
    while ((*handler) (param) == 0) {
        sleep(rendez, lk);
    }
    release(lk);
}

// タイムアウト付きのスリープ
void p9tsleep(Rendez *rendez, struct spinlock *lk, sleephandler_t *handler, void *param, unsigned msecs)
{
    unsigned start = get_ticks();
    acquire(lk);
    while ((*handler) (param) == 0) {
        sleep(rendez, lk);
        if (get_ticks() - start > msecs * HZ / 1000)
            break;
    }
    release(lk);
}

// 起床する
void p9wakeup (Rendez *rendez)
{
    wakeup(rendez);
}

// 0を返す
int return0 (void *param)
{
    return 0;
}

static struct up_t upstruct;
struct up_t *up = &upstruct;


// TODO: カーネルスレッド周りを考える : ether4330.cで2つ作成している
// userdataはタスク（スレッド）固有のデータへのポインタ(struct procに追加?)
void p9proc_init (void)
{
    up->errstr = "";
    initlock(&up->lock, "uplock");
    up->errstack.stackptr = ERROR_STACK_SIZE;
    struct proc *p = thisproc();
    p->userdata = &up->errstack;
}

struct error_stack_t *get_error_stack (void)
{
    struct proc *p = thisproc();

    struct error_stack_t *errstack = (struct error_stack_t *) p->userdata;
    if (errstack == 0) {
        errstack = &up->errstack;
    }

    return errstack;
}
