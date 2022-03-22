// See LICENSE file for copyright and license details.
#pragma once

#include <stdint.h>

#include "capabilities.h"
#include "config.h"
#include "lock.h"

/** Process
 * This is the Process Control Block (PCB). We store pointers
 * register and process's state in this struct.
 */
typedef struct process {
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
        Capability **cap_table;
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
        /** Lock.
         * A core has to claim this lock before running the process.
         * The lock value is zero when unclaimed, non-zero when claimed.
         */
        uintptr_t lock;
} Process;

/** Processes
 * We have a static number of processes N_PROC. The initial process is always
 * process 0.
 */
extern Process processes[N_PROC];

/** Current process
 * This is a core local variable. If the core has a process, then current
 * points to the claimed process, otherwise it is NULL.
 */
register Process *current __asm__("tp");

/** Initialize process.
 * This initializes all processes, setting their PID, stack pointer, and
 * entry points.
 */
void InitProcesses(void);
