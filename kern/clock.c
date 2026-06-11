#include <types.h>
#include <config.h>
#include <clock.h>
#include <arm.h>
#include <base.h>
#include <irq.h>
#include <console.h>
#include <proc.h>
#include <linux/time.h>
#include <rtc.h>
#include <spinlock.h>
#include <linux/errno.h>
#include <net/net.h>

/* ARM 64bit ローカルタイマー : 19.2 MHzのU/Dエッジでカウント */
/* ローカルタイマー割り込みルーティングレジスタ */
#define TIMER_ROUTE             (LOCAL_BASE + 0x24)
#define TIMER_IRQ2CORE(i)       (i)
/* ローカルタイマー制御・ステータスレジスタ */
#define TIMER_CTRL              (LOCAL_BASE + 0x34)
#define TIMER_INTENA            (1 << 29)   /* 割り込みenable */
#define TIMER_ENABLE            (1 << 28)   /* タイマーenable */

/* ローカルタイマーリロードカウント数 */
#if RASPI <= 3
#define TIMER_RELOAD_SEC        (38400000 / HZ)     /* 2 * 19.2 MHz / 100 */
#elif RASPI == 4
#define TIMER_RELOAD_SEC        (108000000)         /* 2 * 54 MHz */
#endif
/* ローカルタイマー割り込みクリア・リロードレジスタ */
#define TIMER_CLR               (LOCAL_BASE + 0x38)
#define TIMER_CLR_INT           (1 << 31)           /* 割り込みフラグクリア */
#define TIMER_RELOAD            (1 << 30)           /* タイマーリロード */

uint64_t tick_usec = TICK_USEC;        /* USER_HZ period (usec) */
uint64_t tick_nsec = TICK_NSEC;        /* ACTHZ period (nsec) */

/* システムタイム (ticks)  */
uint64_t jiffies = 0;
/* 現在時刻 (wall_time) */
struct timespec xtime  __attribute__ ((aligned (16)));
/* 直近のwall_time更新時のjiffies */
unsigned long wall_jiffies = 0;
/* jiffies, xtime, wall_jiffies を保護 */
struct spinlock clocklock;

// 現在時の更新
static void update_wall_time(uint64_t ticks)
{
    do {
        ticks--;
        xtime.tv_nsec += TICK_NSEC;         // 1 tick = 10ms = 10 * 10^6
        if (xtime.tv_nsec >= 1000000000) {  // nsec部分が1秒を超えたら
            xtime.tv_nsec -= 1000000000;    // nsecから1秒引いて
            xtime.tv_sec++;                 // secに1秒足す
        }
    } while (ticks);
}

// 現在時の調整
static inline void update_times(void)
{
    uint64_t ticks;

    ticks = jiffies - wall_jiffies;
    if (ticks) {
        wall_jiffies += ticks;
        update_wall_time(ticks);
    }
}

void clock_init()
{
    put32(TIMER_CTRL, TIMER_INTENA | TIMER_ENABLE | TIMER_RELOAD_SEC);
    put32(TIMER_ROUTE, TIMER_IRQ2CORE(0));
    put32(TIMER_CLR, TIMER_RELOAD | TIMER_CLR_INT);
#ifdef USE_GIC
    irq_enable(IRQ_LOCAL_TIMER);
    irq_register(IRQ_LOCAL_TIMER, clock_intr, 0);
#endif

    initlock(&clocklock, "clock");
    if (rtc_gettime(&xtime) < 0) {
        xtime.tv_nsec = 0L;
        xtime.tv_sec = 1757378760L;      // 2025/09/09 09:46:00 JST
    }
}

static void clock_reset()
{
    put32(TIMER_CLR, TIMER_CLR_INT);
}

/*
 * Real time clock (local timer) interrupt. It gets impluse from crystal clock,
 * thus independent of the variant cpu clock.
 */
void clock_intr()
{
    acquire(&clocklock);
    jiffies++;
    update_times();
    release(&clocklock);
    run_timer_list();
    //acquire(&clocklock);
    clock_reset();
    //wakeup(&jiffies);
    //release(&clocklock);
}

long clock_gettime(clockid_t clk_id, struct timespec *tp)
{
    uint64_t ptime;
    struct proc *p = thisproc();
    static int count = 0;

    switch(clk_id) {
        default:
            return -EINVAL;
        case CLOCK_REALTIME:
            tp->tv_nsec = xtime.tv_nsec;
            tp->tv_sec = xtime.tv_sec;
            break;
        case CLOCK_PROCESS_CPUTIME_ID:
            ptime = (p->stime + p->utime) * TICK_NSEC;
            tp->tv_nsec = ptime % 1000000000;
            tp->tv_sec  = ptime / 1000000000;
            break;
    }
#if 0
    if ((++count % 1000) == 0)
        debug("[%d] clk: %d, tv_sec: %lld, tv_nsec: %lld", count, clk_id, tp->tv_sec, tp->tv_nsec);
#endif
    return 0;
}

long clock_settime(clockid_t clk_id, const struct timespec *tp)
{
    switch(clk_id) {
        default:
            return -EINVAL;
        case CLOCK_REALTIME:
            //if (capable(CAP_SYS_TIME)) {
                xtime.tv_nsec = tp->tv_nsec;
                xtime.tv_sec = tp->tv_sec;
            //} else {
            //    return -EPERM;
            //}
            break;
    }
    return 0;
}

long get_uptime(void)
{
    return jiffies * HZ;
}

long get_ticks(void)
{
    return jiffies;
}

long gettimeofday(struct timeval *tv, void *tz)
{
    (void)tz;
    struct timespec now;
    clock_gettime(CLOCK_REALTIME, &now);

    tv->tv_sec = now.tv_sec;
    tv->tv_usec = now.tv_nsec / 1000;
    return 0;
}

