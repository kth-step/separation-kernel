// See LICENSE file for copyright and license details.
#pragma once

#include "cap.h"
#include "config.h"
#include "types.h"

enum proc_state {
        PS_READY,
        PS_RUNNING,
        PS_WAITING,
        PS_RECEIVING,
        PS_SUSPENDED,
        PS_RUNNING_THEN_SUSPEND,
        PS_SUSPENDED_BUSY,
        PS_RECEIVING_THEN_SUSPEND
};

struct registers {
        uint64_t pc;
        uint64_t ra, sp, gp, tp;
        uint64_t t0, t1, t2;
        uint64_t s0, s1;
        uint64_t a0, a1, a2, a3, a4, a5, a6, a7;
        uint64_t s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
        uint64_t t3, t4, t5, t6;

        uint64_t tpc, tsp, cause, tval;
        uint64_t ppc, psp, pa0, pa1;
};

#define N_REGISTERS (sizeof(struct registers) / sizeof(uint64_t))

struct proc {
        struct registers regs;
        uint64_t pid;
        uint64_t state;
        int64_t channel;
        struct cap_node *cap_table;
};

extern struct proc processes[N_PROC];
register struct proc *current __asm__("tp");

void proc_init(void);

static inline struct cap_node *proc_get_cap_node(struct proc *proc,
                                                 uint64_t cid) {
        if (cid >= N_CAPS)
                return NULL;
        return &proc->cap_table[cid];
}

static inline uint64_t proc_read_register(struct proc *proc, uint64_t regi) {
        uint64_t *regs = (uint64_t *)&proc->regs;
        if (regi < N_REGISTERS) {
                return regs[regi];
        }
        return 0;
}

static inline bool proc_write_register(struct proc *proc, uint64_t regi,
                                       uint64_t regv) {
        uint64_t *regs = (uint64_t *)&proc->regs;
        if (regi < N_REGISTERS) {
                uint64_t tmp = regs[regi];
                regs[regi] = regv;
                return tmp;
        }
        return 0;
}
