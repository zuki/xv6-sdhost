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
#include <linux/mman.h>
#include <mmap.h>
#include <vm.h>

struct spinlock pendinglock;
uint64_t pending;

extern long syscall1(struct trapframe *tf);

static long pf_handler(int ec, int dfs, uint64_t far);
void trap_error(uint64_t type);

#define PSR_MODE_EL0t   0x00000000
#define PSR_MODE_MASK   0x0000000F

#define user_mode(tf) \
    (((int)((tf)->spsr) & PSR_MODE_MASK) == PSR_MODE_EL0t)


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
    int dfs = (int)(iss & 0x3f);

    /* Clear esr. */
    lesr(0);
    switch (ec) {
    case EC_UNKNOWN:
        if (il) {
            trace("IL bit on");
        } else {
            irq_handler(user_mode(tf));
        }
        //check_pending_signal();
        break;

    case EC_SVC64:
        if (iss == 0) {
            tf->x[0] = syscall1(tf);
            //check_pending_signal();
        } else {
            warn("unexpected svc iss 0x%x", iss);
        }
        break;

    case EC_IABORT:     // 0x20 = 32: ユーザモードで発生した命令例外
    case EC_IABORT2:    // 0x21 = 33: カーネルモードで発生した命令例外
        //info("iabort: esr=0x%llx, ec=%d, dfs=%d, far=0x%llx", esr, ec, dfs, far);
        //exit(1);
        // through
    case EC_DABORT:     // 0x24 = 36: ユーザモードで発生したデータ例外
    case EC_DABORT2:    // 0x25 = 37: カーネルモードで発生したデータ例外
        // 4-7: translation fault at level 0-3
        // 8-11: access flag fault at level 0-3
        // 12-15: permission fault at level 0-3
        if (dfs >= 4 && dfs <= 15) {
            if ((tf->x[0] = pf_handler(ec, dfs, far)) < 0) {
                thisproc()->killed = 1;
                info("inst/dataabort: dfs=%d, far=0x%llx", dfs, far);
                exit(1);
            }
            //check_pending_signal();
        } else {
            info("unknown ec: %d, dfs: %d", ec, dfs);
            trap_error(4);
        }
        break;

    default:
        error("[%d] unknown trap code: %d at 0x%llx with 0x%llx", thisproc()->pid, ec, elr, far);
        exit(1);
    }
    softintr();
    check_pending_signal();
}

void
trap_error(uint64_t type)
{
    debug_reg();
    error("irq of type %d unimplemented. \n", type);
    panic("trap_error");
}

static long
pf_handler(int ec, int dfs, uint64_t far)
{
    struct proc *p = thisproc();
    uint64_t *pte;
    struct vma *vma;
    static int count = 0;

    far = ROUNDDOWN(far, PGSIZE);

    if (dfs <= 7) {             // Translation fault
        // 現在のところロジック上ありえない。何らかのバグなのでエラーとする
#if 0
        if (count++ < 2)
            error("[%d] recoveary failed: dfs=%d, far=0x%llx", p->pid, dfs, far);
        return -1;
#endif
        lttbr0((uint64_t)p->pgdir);
        if ((pte = pgdir_walk(p->pgdir, (void *)far, 1)) == 0) {
            if (count++ < 2)
                error("[%d] recoveary failed: ec=0x%x, dfs=%d, far=0x%llx", p->pid, ec, dfs, far);
            return -1;
        } else {
            if (count++ < 4)
                trace("[%d] return 0: ec=0x%x, dfs=%d, far=0x%llx, pte=0x%llx, *pte=0x%llx", p->pid, ec, dfs, far, pte, *pte);
            return 0;
        }
    } else if (dfs <= 11) {     // Access fault: 遅延読み込み
        vma = p->vmas;
        while (vma) {
            if (vma->addr == (void *)far) {
                if (mmap_load_pages(vma->addr, vma->length, vma->prot, vma->flags, vma->f, vma->offset) < 0) {
                    error("load_pages failed: ec=0x%x, dfs=%d, far=0x%llx, vma=0x%p", ec, dfs, far, vma->addr);
                    return -1;
                }
                lttbr0((uint64_t)p->pgdir);
                return 0;
            }
            vma = vma->next;
        }
        return -1;
    } else {                    // Permission fault: Copy on Write
        vma = p->vmas;
        while (vma) {
            if ((uint64_t)vma->addr <= far
             && far < (uint64_t)vma->addr + vma->length
             && vma->flags & MAP_PRIVATE) {
                if (vma->prot & PROT_WRITE) {
                    uint64_t perm = get_perm(vma->prot, vma->flags);
                    perm &= ~PTE_RO;
                    if (copy_vma_pages(vma->addr, vma->length, perm) < 0) {
                        error("copy_vma_pages failed: ec=0x%x, dfs=%d, far=0x%llx, vma=0x%p, perm=0x%llx", ec, dfs, far, vma->addr, perm);
                        return -1;
                    }
                    lttbr0((uint64_t)p->pgdir);
                    return 0;
                }
            }
            vma = vma->next;
        }
        return -1;
    }
}
