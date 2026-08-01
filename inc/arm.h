#ifndef INC_ARM_H
#define INC_ARM_H

#include <types.h>

/* Prevent compiler from reordering. */
static inline void
barrier()
{
    __asm__ __volatile__("" ::: "memory");
}

/* cntfrq_el0 = 19200000 : armstub8.Sで設定 */
static inline uint64_t
timerfreq()
{
    uint64_t f;
    __asm__ __volatile__ ("mrs %[freq], cntfrq_el0" : [freq]"=r"(f));
    return f;
}

static inline uint64_t
timestamp()
{
    uint64_t t;
    barrier();
    __asm__ __volatile__ ("mrs %[cnt], cntpct_el0" : [cnt]"=r"(t));
    barrier();
    return t;
}

/* Wait n CPU cycles. */
static inline void
delay(uint32_t n)
{
    __asm__ __volatile__("__delay_%=: subs %[n], %[n], #1; bne __delay_%=\n":
                 "=r"(n): [n]"0"(n) : "cc");
}

/* Wait n microsec. */
static inline void
delayus(uint32_t n)
{
    uint64_t f = timerfreq(), t = timestamp(), r;
    /* Calculate expire value for counter. */
    t += f / 1000000 * n;
    do {
        r = timestamp();
    } while (r < t);
}

/* Instruction synchronization barrier. */
static inline void
isb()
{
    __asm__ __volatile__("isb" ::: "memory");
}

/* Data synchronization barrier. */
static inline void
dsb()
{
    __asm__ __volatile__("dsb sy" ::: "memory");
}

/* Data memory barrier. */
static inline void
dmb()
{
    __asm__ __volatile__("dmb sy" ::: "memory");
}

/* Brute-force data and instruction synchronization barrier. */
static inline void
disb()
{
    dsb();
    isb();
}

/* Don't need dmb since device region is marked as nGnRnE by kpgdir. */
static inline void
put32(uint64_t p, uint32_t x)
{
    barrier();
    *(__volatile__ uint32_t *)p = x;
    barrier();
}

static inline uint32_t
get32(uint64_t p)
{
    barrier();
    uint32_t val = *(__volatile__ uint32_t *)p;
    barrier();
    return val;
}


/* Data cache clean and invalidate by virtual address to point of coherency. */
static inline void
dccivac(void *p, int n)
{
    disb();
    while (n--) {
        __asm__ __volatile__("dc civac, %[x]" : : [x]"r"(p + n));
        __asm__ __volatile__("dc cvau, %[x]" : : [x]"r"(p + n));
    }
    disb();
}

/* Read Exception Syndrome Register (EL1) */
static inline uint64_t
resr()
{
    disb();
    uint64_t r;
    __asm__ __volatile__("mrs %[x], esr_el1" : [x]"=r"(r));
    disb();
    return r;
}

/* Read Exception Link Register (EL1) */
static inline uint64_t
relr()
{
    disb();
    uint64_t r;
    __asm__ __volatile__("mrs %[x], elr_el1" : [x]"=r"(r));
    disb();
    return r;
}

/* Load Exception Syndrome Register (EL1) */
static inline void
lesr(uint64_t _v)
{
    disb();
    __asm__ __volatile__("msr esr_el1, %[x]" : : [x]"r"(0));
    disb();
}

/* Read Fault Address Register (EL1) */
static inline uint64_t
rfar()
{
    disb();
    uint64_t r;
    __asm__ __volatile__("mrs %[x], far_el1" : [x]"=r"(r));
    disb();
    return r;
}

/* Load vector base (virtual) address register (EL1) */
static inline void
lvbar(void *p)
{
    disb();
    __asm__ __volatile__("msr vbar_el1, %[x]" : : [x]"r"(p));
    disb();
}

static inline void
tlbi1()
{
    disb();
    __asm__ __volatile__("tlbi vmalle1is");
    disb();
}

/* Load Translation Table Base Register 0 (EL1) */
static inline void
lttbr0(uint64_t p)
{
    disb();
    __asm__ __volatile__("msr ttbr0_el1, %[x]" : : [x]"r"(p));
    tlbi1();
}

/* Load Translation Table Base Register 1 (EL1) */
static inline void
lttbr1(uint64_t p)
{
    disb();
    __asm__ __volatile__("msr ttbr1_el1, %[x]" : : [x]"r"(p));
    tlbi1();
}

static inline int
cpuid()
{
    int64_t id;
    __asm__ __volatile__("mrs %[x], mpidr_el1" : [x]"=r"(id));
    return id & 0xFF;
}

static inline void
nop()
{
    __asm__ __volatile__ ("nop");
}

static inline void
wfi()
{
    __asm__ __volatile__ ("wfi");
}

static inline void
wfe()
{
    __asm__ __volatile__ ("wfe");
}

static inline void
sev()
{
    __asm__ __volatile__ ("sev");
}

static inline void
enable_irq(void)
{
    __asm__ __volatile__ ("msr DAIFClr, #2");
}

static inline void
disable_irq(void)
{
    __asm__ __volatile__ ("msr DAIFSet, #2");
}

static inline void
enable_fiq(void)
{
    __asm__ __volatile__ ("msr DAIFClr, #1");
}

static inline void
disable_fiq(void)
{
    __asm__ __volatile__ ("msr DAIFSet, #1");
}

static inline int
irq_enabled(void)
{
    uint64_t r;
    __asm__ __volatile__("mrs %[x], daif" : [x]"=r"(r));
    return (r & 0x80) == 0;
}

static inline uint64_t get_sp(void)
{
    uint64_t x;
    __asm__ __volatile__("mov %0, sp" : "=r" (x) );
    return x;
}

#endif
