// See LICENSE file for copyright and license details.
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cap_node.h"
#include "config.h"
#include "s3k_consts.h"

#define N_REGISTERS (sizeof(registers_t) / sizeof(uint64_t))

typedef enum proc_state proc_state_t;
typedef struct registers registers_t;
typedef struct proc proc_t;

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
        /* Standard registers */
        uint64_t pc;
        uint64_t ra, sp, gp, tp;
        uint64_t t0, t1, t2;
        uint64_t s0, s1;
        uint64_t a0, a1, a2, a3, a4, a5, a6, a7;
        uint64_t s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
        uint64_t t3, t4, t5, t6;
        /* timer */
        uint64_t timeout;
        /* Exception handling registers */
        uint64_t tpc, tsp, cause, tval;
        uint64_t ppc, psp, pa0, pa1;
};

struct proc {
        registers_t regs;
        uint64_t pid;
        uint64_t state;
        cap_node_t* cap_table;
};

extern proc_t processes[N_PROC];
register proc_t* current __asm__("tp");

void proc_init(void);

static inline cap_node_t * proc_get_cap_node(proc_t* proc, uint64_t cid);
static inline cap_t proc_get_cap(proc_t* proc, uint64_t cid);
static inline uint64_t proc_read_register(proc_t* proc, uint64_t regi);
static inline uint64_t proc_write_register(proc_t* proc, uint64_t regi, uint64_t regv);
static inline void proc_release(proc_t* proc);
static inline bool proc_acquire(proc_t* proc);
static inline bool proc_is_suspended(proc_t* proc);
static inline bool proc_supervisor_acquire(proc_t* proc);
static inline void proc_supervisor_release(proc_t* proc);
static inline bool proc_supervisor_resume(proc_t* proc);
static inline bool proc_supervisor_suspend(proc_t* proc);
static inline bool proc_receiver_wait(proc_t* proc, uint64_t channel);
static inline bool proc_sender_acquire(proc_t* proc, uint64_t channel);
static inline void proc_sender_release(proc_t* proc);

cap_node_t* proc_get_cap_node(proc_t* proc, uint64_t cid)
{
        return &proc->cap_table[cid % N_CAPS];
}

cap_t proc_get_cap(proc_t* proc, uint64_t cid)
{
        return cap_node_get_cap(proc_get_cap_node(proc, cid));
}

uint64_t proc_read_register(proc_t* proc, uint64_t regi)
{
        uint64_t* regs = (uint64_t*)&proc->regs;
        if (regi < N_REGISTERS) {
                return regs[regi];
        }
        return 0;
}

uint64_t proc_write_register(proc_t* proc, uint64_t regi, uint64_t regv)
{
        uint64_t* regs = (uint64_t*)&proc->regs;
        if (regi < N_REGISTERS) {
                uint64_t tmp = regs[regi];
                regs[regi] = regv;
                return tmp;
        }
        return 0;
}

void proc_release(proc_t* proc)
{
        if (!(proc->state & PROC_STATE_WAITING)) {
                /* If not waiting, then go to suspend */
                __sync_fetch_and_and(&proc->state, PROC_STATE_SUSPENDED);
        }
}

bool proc_acquire(proc_t* proc)
{
        return __sync_bool_compare_and_swap(&proc->state, PROC_STATE_READY, PROC_STATE_RUNNING);
}

bool proc_is_suspended(proc_t* proc)
{
        return (proc->state & PROC_STATE_SUSPENDED) != 0;
}

bool proc_supervisor_acquire(proc_t* proc)
{
        return __sync_bool_compare_and_swap(&proc->state, PROC_STATE_SUSPENDED, PROC_STATE_SUSPENDED_BUSY);
}

void proc_supervisor_release(proc_t* proc)
{
        kassert(proc->state == PROC_STATE_SUSPENDED_BUSY);
        __sync_synchronize();
        proc->state = PROC_STATE_SUSPENDED;
}

bool proc_supervisor_resume(proc_t* proc)
{
        if (proc_supervisor_acquire(proc)) {
                proc->state = PROC_STATE_READY;
                return S3K_OK;
        }
        return S3K_SUPERVISEE_BUSY;
}

bool proc_supervisor_suspend(proc_t* proc)
{
        /* Set the suspended bit */
        enum proc_state state = __sync_fetch_and_or(&proc->state, PROC_STATE_SUSPENDED);
        if ((state & 7) == PROC_STATE_WAITING) {
                proc->state = PROC_STATE_SUSPENDED;
        }
        return S3K_OK;
}

bool proc_receiver_wait(proc_t* proc, uint64_t channel)
{
        uint64_t expected = PROC_STATE_RUNNING;
        uint64_t desired = channel << 48 | PROC_STATE_WAITING;
        return __sync_bool_compare_and_swap(&current->state, expected, desired);
}

bool proc_sender_acquire(proc_t* proc, uint64_t channel)
{
        uint64_t expected = channel << 48 | PROC_STATE_WAITING;
        uint64_t desired = channel << 48 | PROC_STATE_RECEIVING;
        return __sync_bool_compare_and_swap(&proc->state, expected, desired);
}

void proc_sender_release(proc_t* proc)
{
        __sync_fetch_and_and(&proc->state, PROC_STATE_SUSPENDED);
}
