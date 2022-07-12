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
        uint64_t  ppc, psp, pa0, pa1;
        /* Points to pmp entries */
        uint64_t pmp0;
};

#define PROC_NUM_OF_REGS (sizeof(struct trap_frame) / sizeof(uint64_t))

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
