// From https://github.com/pandax381/xv6-riscv-net
#ifndef INC_NET_PLATFORM_H
#define INC_NET_PLATFORM_H

#include <types.h>
#include <mm.h>
#include <spinlock.h>
#include <proc.h>
#include <trap.h>
#include <linux/time.h>
#include <linux/errno.h>

/*
 * Memory
 */
//TODO slab_allocの使用を検討

#define memory_alloc(size)  kmalloc(size)

#define memory_free(ptr)    kmfree(ptr)


/*
 * Mutex
 */

typedef struct spinlock mutex_t;

#define MUTEX_INITIALIZER {0}

static inline int mutex_init(mutex_t *mutex, char *name)
{
    initlock(mutex, name);
    return 0;
}

static inline int mutex_lock(mutex_t *mutex)
{
    acquire(mutex);
    return 0;
}

static inline int mutex_unlock(mutex_t *mutex)
{
    release(mutex);
    return 0;
}

/*
 * Interrupt
 */
#include <softirq.h>

#define INTR_IRQ_SOFTIRQ    SOFT_IRQ_NET_RX
#define INTR_IRQ_EVENT      SOFT_IRQ_NET_EVENT

static inline int intr_raise_irq(unsigned int irq)
{
    acquire(&pendinglock);
    pending |= irq;
    release(&pendinglock);
    return 0;
}

static inline int intr_init(void)
{
    return 0;
}

static inline int intr_run(void)
{
    return 0;
}

static inline void intr_shutdown(void)
{
    return;
}

/*
 * Scheduler
 */

struct sched_ctx {
    int interrupted;    // struct synchro_event->state 相当
    int wc;             /* wait count : wait_list_head 代替 */
};

#define SCHED_CTX_INITIALIZER {0, 0}

// CSynchronizationEvent コンストラクタ相当
static inline int sched_ctx_init(struct sched_ctx *ctx)
{
    ctx->interrupted = 0;
    ctx->wc = 0;
    return 0;
}

static inline int sched_ctx_destroy(struct sched_ctx *ctx)
{
    if (ctx->wc) {
        return -1;
    }
    return 0;
}

// CSynchronizationEvent::Clear 相当がないが、常に Clear(); Wait()の形で使われており、
// これは sched_sleep()で代用できそう
// CSynchronizationEvent::Wait 相当
static inline int sched_sleep(struct sched_ctx *ctx, mutex_t *mutex, const struct timespec *abstime)
{
    (void)abstime;
    if (ctx->interrupted) {
        return -EINTR;
    }
    ctx->wc++;
    sleep(ctx, mutex);
    ctx->wc--;
    if (ctx->interrupted) {
        if (!ctx->wc) {
            ctx->interrupted = 0;
            dsb();
        }
        return -EINTR;
    }
    return 0;
}

// CSynchronizationEvent::Pulse 相当
static inline int sched_wakeup(struct sched_ctx *ctx)
{
    ctx->interrupted = 0;
    dsb();
    wakeup(ctx);
    return 0;
}

// CSynchronizationEvent::Set 相当
static inline int sched_interrupt(struct sched_ctx *ctx)
{
    ctx->interrupted = 1;
    dsb();
    wakeup(ctx);
    return 0;
}

#endif
