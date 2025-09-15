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

void
trap_init()
{
    extern char vectors[];
    lvbar(vectors);
    lesr(0);
}

void
trap(struct trapframe *tf)
{
    uint64_t esr = resr();
    uint64_t far = rfar();
    int ec  = (int)(esr >> EC_SHIFT);
    int iss = (int)(esr & ISS_MASK);
    int il  = (int)(esr & IR_MASK);
    int dfs = (int)(iss & 0x3f);

    /* Clear esr. */
    lesr(0);
    switch (ec) {
    case EC_UNKNOWN:
        if (il) {
            debug("IL bit on");
        } else
            irq_handler();
        break;

    case EC_SVC64:
        if (iss == 0) {
            tf->x[0] = syscall1(tf);
        } else {
            warn("unexpected svc iss 0x%x", iss);
        }
        break;

    default:
        info("unknown trap code: %d", ec);
        exit(1);
    }
}

void
trap_error(uint64_t type)
{
    debug_reg();
    panic("irq of type %d unimplemented. \n", type);
}
