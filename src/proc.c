// See LICENSE file for copyright and license details.
#include "proc.h"

#include <stddef.h>

#include "atomic.h"
#include "config.h"
#include "stack.h"

/** Initial stack offset.
 * The initialization of a process is a bit awkward, we basically
 * start in the middle of a function call, for this we have this
 * stack offset which should be safe. It must fit all registers saved
 * to the stack in switch.S and entry.S.
 */
#define INIT_STACK_OFFSET 32

/** AsmTrapExit
 * This assembly function goes to user mode.
 */
extern void AsmTrapExit();

/* Defined in proc.h */
Process processes[N_PROC];
Capability cap_tables[N_PROC][N_CAPS];

/* Initializes one process. */
static void proc_init_proc(int pid) {
        /* Get the PCB */
        Process *proc = processes + pid;
        /* Set the process id to */
        proc->pid = pid;
        /* Set the process's kernel stack. */
        proc->ksp = &proc_stack[pid][STACK_SIZE / 8 - INIT_STACK_OFFSET];
        /* Set the return address located as first element on the stack. The
         * kernel starts, it will load the return address from the top of
         * process stack, and jump to that address to service the process.
         */
        *proc->ksp = (uintptr_t)AsmTrapExit;
        /* Set the user program counter, this will be the entry point. */
        proc->pc = (uintptr_t *)user_code;
}

/* Defined in proc.h */
void ProcInitProcesses(void) {
        /* Initialize processes. */
        for (int i = 0; i < N_PROC; i++)
                proc_init_proc(i);
}
