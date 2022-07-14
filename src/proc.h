// See LICENSE file for copyright and license details.
#pragma once

#include "cap_node.h"
#include "config.h"
#include "types.h"

enum proc_state {
        PROC_STATE_READY,
        PROC_STATE_RUNNING,
        PROC_STATE_WAITING,
        PROC_STATE_RECEIVING,
        PROC_STATE_SUSPENDED,
        PROC_STATE_RUNNING_THEN_SUSPEND,
        PROC_STATE_SUSPENDED_BUSY,
        PROC_STATE_RECEIVING_THEN_SUSPEND
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
        struct cap_node* cap_table;
};

extern struct proc processes[N_PROC];
register struct proc* current __asm__("tp");

void proc_init(void);

static inline struct cap_node* proc_get_cap_node(struct proc* proc, uint64_t cid)
{
        if (cid >= N_CAPS)
                return NULL;
        return &proc->cap_table[cid];
}

static inline uint64_t proc_read_register(struct proc* proc, uint64_t regi)
{
        uint64_t* regs = (uint64_t*)&proc->regs;
        if (regi < N_REGISTERS) {
                return regs[regi];
        }
        return 0;
}

static inline bool proc_write_register(struct proc* proc, uint64_t regi, uint64_t regv)
{
        uint64_t* regs = (uint64_t*)&proc->regs;
        if (regi < N_REGISTERS) {
                uint64_t tmp = regs[regi];
                regs[regi] = regv;
                return tmp;
        }
        return 0;
}

static inline void proc_release(struct proc* proc)
{
        if (proc == NULL)
                return;
        if (!(proc->state & PROC_STATE_WAITING)) {
                /* If not waiting, then go to suspend */
                __sync_fetch_and_and(&proc->state, PROC_STATE_SUSPENDED);
        }
}

static inline bool proc_acquire(struct proc* proc)
{
        return __sync_bool_compare_and_swap(&proc->state, PROC_STATE_READY, PROC_STATE_RUNNING);
}

static inline bool proc_is_suspended(struct proc* proc)
{
        return (proc->state & PROC_STATE_SUSPENDED) != 0;
}

static inline bool proc_supervisor_acquire(struct proc* proc)
{
        return __sync_bool_compare_and_swap(&proc->state, PROC_STATE_SUSPENDED, PROC_STATE_SUSPENDED_BUSY);
}

static inline void proc_supervisor_release(struct proc* proc)
{
        kassert(proc->state == PROC_STATE_SUSPENDED_BUSY);
        __sync_synchronize();
        proc->state = PROC_STATE_SUSPENDED;
}

static inline bool proc_supervisor_resume(struct proc* proc)
{
        if (proc_supervisor_acquire(proc)) {
                proc->state = PROC_STATE_READY;
                return true;
        }
        return false;
}

static inline bool proc_supervisor_suspend(struct proc* proc)
{
        /* Set the suspended bit */
        enum proc_state state = __sync_fetch_and_or(&proc->state, PROC_STATE_SUSPENDED);
        if (state & PROC_STATE_SUSPENDED) /* Already suspended */
                return false;

        if (state == PROC_STATE_WAITING) {
                proc->channel = -1;
                __sync_synchronize();
                proc->state = PROC_STATE_SUSPENDED;
        }
        return true;
}

static inline bool proc_goto_waiting(struct proc* proc)
{
        return __sync_bool_compare_and_swap(&current->state, PROC_STATE_RUNNING, PROC_STATE_WAITING);
}

static inline bool proc_sender_acquire(struct proc* proc)
{
        while (1) {

                if (proc->state & PROC_STATE_SUSPENDED)
                        return false;
                if (__sync_bool_compare_and_swap(&proc->state, PROC_STATE_WAITING, PROC_STATE_RECEIVING))
                        return true;
        }
}

static inline void proc_sender_release(struct proc* proc) {
        __sync_fetch_and_and(&proc->state, PROC_STATE_SUSPENDED);
}

