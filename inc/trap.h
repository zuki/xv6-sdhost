#ifndef INC_TRAP_H
#define INC_TRAP_H

#include <types.h>
#include <spinlock.h>

struct trapframe {
    uint64_t spsr, elr, sp, tpidr;
    uint64_t x[31];
    uint64_t padding;
    uint64_t q0[2]; // FIXME: dirty hack since musl's `memset` only used q0.
};

void trap(struct trapframe *);
void trap_init();
void dump_tf(struct trapframe *tf);

extern uint64_t         pending;
extern struct spinlock  pendinglock;

#endif
