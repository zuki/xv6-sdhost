#include <trap.h>
#include <arm.h>
#include <types.h>
#include <sysregs.h>
#include <mmu.h>
#include <irq.h>
#include <memlayout.h>
#include <console.h>
#include <proc.h>
#include <debug.h>
#include <spinlock.h>
#include <softirq.h>
#include <net/net.h>

struct spinlock pendinglock;
uint64_t pending;

extern long syscall1(struct trapframe *tf);

static void softintr(void)
{
    acquire(&pendinglock);
    uint64_t irqs = pending;
    pending = 0;
    release(&pendinglock);

    if (irqs) {
        trace("irqs: 0x%x", irqs);
    } else {
        return;
    }

    if (irqs & SOFT_IRQ_NET_RX) {
        net_softirq_handler();
    }
    if (irqs & SOFT_IRQ_NET_EVENT) {
        net_event_handler();
    }
}

void trap_init()
{
    extern char vectors[];
    lvbar(vectors);
    lesr(0);
    info("[%d] trap_init ok", cpuid());
}

void trap(struct trapframe *tf)
{
    uint64_t esr = resr();
    uint64_t far = rfar();
    uint64_t elr = relr();
    int ec  = (int)(esr >> EC_SHIFT);
    int iss = (int)(esr & ISS_MASK);
    int il  = (int)(esr & IR_MASK);
    //int dfs = (int)(iss & 0x3f);

    /* Clear esr. */
    lesr(0);
    switch (ec) {
    case EC_UNKNOWN:
        if (il) {
            trace("IL bit on");
        } else {
            irq_handler();
        }
        break;

    case EC_SVC64:
        if (iss == 0) {
            tf->x[0] = syscall1(tf);
        } else {
            warn("unexpected svc iss 0x%x", iss);
        }
        break;

    default:
        info("unknown trap code: %d at 0x%llx with 0x%llx", ec, elr, far);
        exit(1);
    }
    softintr();
}

void
trap_error(uint64_t type)
{
    debug_reg();
    error("irq of type %d unimplemented. \n", type);
    panic("trap_error");
}
