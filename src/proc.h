// See LICENSE file for copyright and license details.
#pragma once

#include "cap.h"
#include "config.h"
#include "types.h"

#define PROC_NUM_OF_REGS 37

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
        /* Exception handling registers */
        uint64_t cause, tval, epc, esp;

        /* Points to pmp entries */
        uint64_t pmp0;
};

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
        char *ksp;

        /** Process identifier.
         * This is the process's ID used for identification during
         * inter-process communication.
         */
        uint64_t pid;

        TrapFrame *tf;

        /** Capability table.
         * Pointer to the capability table.
         */
        CapNode *cap_table;

        /* The pmp configurations are stored in these capabilities */
        /* pmp_table[i].data[1] = pmpicfg | pmpaddri */
        CapNode pmp_table[8];

        ProcState state;
};

/** Processes
 * We have a static number of processes N_PROC. The initial process is always
 * process 0.
 */
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
