#include <types.h>
#include <config.h>
#include <timer.h>
#include <arm.h>
#include <base.h>
#include <irq.h>
#include <console.h>
#include <mm.h>
#include <proc.h>
#include <spinlock.h>
#include <slab.h>
#include <linux/time.h>

/* コア(n)割り込み制御レジスタ : 0x4000_0040 + (4 * n) */
#define CORE_TIMER_CTRL(i)      (LOCAL_BASE + 0x40 + 4*(i))
#define CORE_TIMER_ENABLE       (1 << 1)        /* CNTPNSIRQ enable */

/* タイマー設定カウント値 */
static uint64_t dt;

/* timer_listを保護するロック */
struct spinlock timerlock;
/* タイマーリスト: コアごとに存在 */
static struct timer_list timer_list;
/* 次のタイマー発火までのticks */
static uint64_t timer_jiffies = 0;

static struct slab_cache *TIMERS;

void timer_init()
{
    // 1秒で発火
#ifdef USING_RASPI
    dt = timerfreq();       // 10 ms = 19.2 * 10^6 / 100
#else
    dt = 62500000UL;        // QEMUはtimerfreq()で得られる値が実機と違う
#endif
    trace("timerfreq = 0x%llx", timerfreq());
    //dt = timerfreq();     /* dt = 19_200_000 */
    asm volatile ("msr cntp_ctl_el0, %[x]"::[x] "r"(1));    /* タイマーenable */
    asm volatile ("msr cntp_tval_el0, %[x]"::[x] "r"(dt));
    put32(CORE_TIMER_CTRL(cpuid()), CORE_TIMER_ENABLE);
#ifdef USE_GIC
    irq_enable(IRQ_LOCAL_CNTPNS);
    irq_register(IRQ_LOCAL_CNTPNS, timer_intr, 0);
#endif

    initlock(&timerlock, "timer");
    list_init(&timer_list.list);
    TIMERS = slab_cache_create("timer_list", sizeof(struct timer_list), 64);
}

static void timer_reset()
{
    asm volatile ("msr cntp_tval_el0, %[x]"::[x] "r"(dt));
}

/*
 * 現在はタイマーをリロードするだけで特に何もしていない。
 */
void timer_intr()
{
    timer_reset();
    //yield();
}

/* タイマーリストに追加し、発火時間を再計算する */
static inline void internal_add_timer(struct timer_list *timer)
{
    //割り込み禁止（trap.c）&& timerlockを保持

    struct list_head *head, *curr;
    struct timer_list *entry;
    boolean unset = true;

    head = &timer_list.list;
    curr = head->next;

    while (curr != head) {
        entry = list_entry(curr, struct timer_list, list);
        if (timer->expires <= entry->expires) {
            if (curr->prev == head) {           // entryは先頭
                entry->expires -= timer->expires;
                list_push_front(&timer_list.list, &timer->list);
            } else if (curr->next == head) {    // entryは末尾
                timer->expires -= entry->expires;
                list_push_back(&timer_list.list, &timer->list);
            } else {                            // entryの前に挿入
                entry->expires -= timer->expires;
                list_insert(&timer->list, entry->list.prev, &entry->list);
            }
            unset = false;
            break;
        }
        timer->expires -= entry->expires;
        curr = curr->next;
    }
    if (unset)
        list_push_back(&timer_list.list, &timer->list);
}

#if 0
static void update_proc_time(int user_mode)
{
    struct proc *p = thisproc();
    if (user_mode)
        p->utime++;
    else
        p->stime++;
}
#endif

struct timer_list *alloc_timer(void)
{
    struct timer_list *timer = (struct timer_list *)slab_cache_alloc(TIMERS);
    timer->list.next = timer->list.prev = NULL;
    return timer;
}

void free_timer(struct timer_list *timer) {
    slab_cache_free(TIMERS, timer);
}

/* タイマーを登録から外す */
static inline int detach_timer(struct timer_list *timer)
{
    if (!timer_pending(timer))  // 未登録
        return 0;
    list_drop(&timer->list);    // リストから自分を外す
    return 1;
}

/* タイマーを追加する */
void add_timer(struct timer_list *timer)
{
    acquire(&timerlock);
    if (timer_pending(timer))   // 二重登録
        goto bug;
    internal_add_timer(timer);
    release(&timerlock);
    return;
bug:
    release(&timerlock);
    warn("bug: kernel timer added twice at %p",
            __builtin_return_address(0));
}

// TODO: return値がokの場合、1になっているのを修正
/* タイアーを削除する */
int del_timer(struct timer_list * timer)
{
    int ret;

    acquire(&timerlock);
    ret = detach_timer(timer);                      // 1. リストから削除
    timer->list.next = timer->list.prev = NULL;     // 2. 内部リストからクリア
    release(&timerlock);
    if (timer->data != 0)
        kmfree((void *)timer->data);
    free_timer(timer);
    return ret;
}

/* 時限が来たタイマーを実行した後、タイマーリストを更新する */
void run_timer_list(void)
{
    struct list_head *head, *curr;
    uint64_t delta = jiffies - timer_jiffies;

    acquire(&timerlock);
    head = &timer_list.list;
    curr = head->next;

    while (curr != head) {
        struct timer_list *timer;
        void (*fn)(uint64_t);
        uint64_t data;              // データには(void *)が設定可能

        timer = list_entry(curr, struct timer_list, list);
        if (timer->expires <= delta) {
            fn = timer->fn;
            data = timer->data;
            detach_timer(timer);
            timer->list.next = timer->list.prev = NULL;
            release(&timerlock);
            fn(data);
            acquire(&timerlock);
        } else {
            timer->expires -= delta;    // リストの先頭のexpiresを経過時刻だけ減ずる
            break;
        }
        curr = curr->next;
    }
    timer_jiffies += delta;
    release(&timerlock);
}
