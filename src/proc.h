// See LICENSE file for copyright and license details.
#pragma once

#include <stdint.h>

#include "capabilities.h"
#include "config.h"
#include "lock.h"

/** Proc
 * This is the Process Control Block (PCB). We store pointers
 * register and process's state in this struct.
 */
typedef struct proc Proc;
/** ProcState
 * Describes the state of a process.
 */
typedef enum proc_state ProcState;


enum proc_state {
        PROC_HALTED, PROC_SUSPENDED, PROC_RUNNING, PROC_BLOCKED
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
        uintptr_t *ksp;
        /** Process identifier.
         * This is the process's ID used for identification during
         * inter-process communication.
         */
        uintptr_t pid;
        /** Capability table.
         * Pointer to the capability table.
         */
        Capability *cap_table;
        /** Argument registers.
         * We store the argument registers a0-a7 in the args array,
         * this simplifies inter-process communication.
         */
        uintptr_t args[8];
        /** Program counter.
         * We store the process's program counter here, this simplifies
         * exception handling and supervisor capabilities.
         */
        uintptr_t *pc;
        /** Process state.
         * TODO: Comment
         */
        ProcState state;
        /* TODO: Replace with process state */
        Lock lock;
};


/** Processes
 * We have a static number of processes N_PROC. The initial process is always process 0.
 */
extern Proc processes[N_PROC];
/** Capability table */
extern Capability cap_tables[N_PROC][N_CAPS];

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
