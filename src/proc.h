// See LICENSE file for copyright and license details.
#pragma once

#include "cap.h"
#include "config.h"
#include "types.h"

/** Proc
 * This is the Process Control Block (PCB). We store pointers
 * register and process's state in this struct.
 */
typedef struct proc Proc;
/** ProcState
 * Describes the state of a process.
 */
typedef enum proc_state ProcState;

typedef struct trap_frame TrapFrame;

enum proc_state {
        PROC_READY,
        PROC_RUNNING,
        PROC_WAITING,
        PROC_RECEIVING,
        PROC_SUSPENDED,
        PROC_RUNNING_THEN_SUSPEND,
        PROC_SUSPENDED_BUSY,
        PROC_RECEIVING_THEN_SUSPEND
};

struct trap_frame {
        /* Kernel registers */
        uint64_t kgp, ktp, mstatus, mscratch;

        /* Process registers */
        uint64_t pc;
        uint64_t ra, sp, gp, tp;
        uint64_t t0, t1, t2;
        uint64_t s0, s1;
        uint64_t a0, a1, a2, a3, a4, a5, a6, a7;
        uint64_t s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
        uint64_t t3, t4, t5, t6;
        /* Trap pc and trap sp. */
        uint64_t tpc, tsp;
        /* Previous pc, sp, a0 and a1 (before exception). */
        uint64_t ppc, psp, pa0, pa1;
        /* Points to pmp entries */
        uint64_t pmp0;
};

#define TF_PROCESS_REGISTERS \
        ((sizeof(TrapFrame) - __builtin_offsetof(struct trap_frame, pc))/sizeof(uint64_t))

struct proc {
        /** Kernel stack pointer.
         *
         * When a process is running, we store the core's stack pointer
         * (pointer to core_stack) to ksp.
         *
         * When the process is not running, i.e., when the scheduler or some
         * other process is running, then we store the process's stack pointer
         * (pointer to proc_stack) to ksp.
         */
        void *ksp;
        /* Process identifier */
        uint64_t pid;
        /* IPC channel subscription, -1 == not listening */
        int64_t channel;
        /* Registers */
        TrapFrame *tf;
        /* Capability table */
        CapNode *cap_table;
        /* Process state */
        ProcState state;
};

/* Processes, process 0 is boot process. */
extern Proc processes[N_PROC];

/** Current process
 * This is a core local variable. If the core has a process, then current
 * points to the claimed process, otherwise it is NULL.
 */
register Proc *current __asm__("tp");

/** Initialize process.
 * This initializes all processes, setting their PID, stack pointer, and
 * entry points.
 */
void ProcInitProcesses(void);

static inline CapNode *proc_get_cap_node(Proc *proc, uint64_t cid) {
        if (cid >= N_CAPS)
                return NULL;
        return &proc->cap_table[cid];
}

static inline void proc_reset_stack(Proc *proc) {
        proc->ksp = (void *)proc->tf;
}

/**
 * Set the process to suspend.
 */
static inline ProcState proc_halt(Proc *proc) {
        ProcState state = __sync_fetch_and_or(&proc->state, PROC_SUSPENDED);
        if (state == PROC_WAITING) {
                /* Process is waiting for IPC, we must deque it */
                proc->channel = -1;
                __sync_synchronize();
                proc->state = PROC_SUSPENDED;
        }
        return (state & PROC_SUSPENDED) == 0;
}

static inline ProcState proc_resume_from_receive(Proc *proc) {
        return __sync_fetch_and_and(&proc->state, PROC_SUSPENDED);
}

static inline ProcState proc_resume_from_suspend(Proc *proc) {
        ProcState state = __sync_val_compare_and_swap(&proc->state, PROC_SUSPENDED, PROC_SUSPENDED_BUSY);
        if (state == PROC_SUSPENDED) {
                proc_reset_stack(proc);
                __sync_synchronize();
                proc->state = PROC_READY;
        }
        return state;
}

static inline uint64_t proc_read_register(Proc *proc, uint64_t regid) {
        uint64_t *regs = (uint64_t*) &proc->tf->pc;
        if (regid < TF_PROCESS_REGISTERS) {
                return regs[regid];
        }
        return 0;
}

static inline uint64_t proc_write_register(Proc *proc, uint64_t regid, uint64_t value) {
        uint64_t *regs = (uint64_t*) &proc->tf->pc;
        if (regid < TF_PROCESS_REGISTERS) {
                uint64_t tmp = regs[regid];
                regs[regid] = value;
                return tmp;
        }
        return 0;
}
