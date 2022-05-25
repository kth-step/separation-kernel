// See LICENSE file for copyright and license details.
#include "proc.h"

#include <stddef.h>

#include "cap.h"
#include "cap_util.h"
#include "config.h"
#include "stack.h"

/** Initial stack offset.
 * The initialization of a process is a bit awkward, we basically
 * start in the middle of a function call, for this we have this
 * stack offset which should be safe. It must fit all registers saved
 * to the stack in switch.S and entry.S.
 */
#define INIT_STACK_OFFSET 32

/* Temporary. */
extern void user_code();

/* Defined in proc.h */
Proc processes[N_PROC];

/* Initializes one process. */
static void proc_init_proc(int pid) {
        /* Get the PCB */
        Proc *proc = processes + pid;
        /* Set the process id to */
        proc->pid = pid;
        /* Set the process's kernel stack. */
        proc->ksp = &proc_stack[pid][STACK_SIZE / 8 - INIT_STACK_OFFSET];
        /* Set the capability table. */
        proc->cap_table = cap_tables[pid];
        for (int i = 0; i < N_CAPS; ++i) {
                proc->cap_table[i].next = NULL;
                proc->cap_table[i].prev = NULL;
        }
        /* Set process to HALTED. */
        proc->state = PROC_HALTED;
}

static void proc_init_boot_proc(Proc *boot) {
        /* Set the initial PC. */
        // boot->pc = (uintptr_t)(pe_begin << 2);
        *boot->ksp = (uintptr_t)user_code;  // Temporary code.

        /* Set boot process to running. */
        boot->state = PROC_SUSPENDED;
}

/* Defined in proc.h */
void ProcInitProcesses(void) {
        /* Initialize processes. */
        for (int i = 0; i < N_PROC; i++)
                proc_init_proc(i);
        /*** Boot process ***/
        proc_init_boot_proc(&processes[0]);
}

void ProcHalt(Proc *proc) {
        proc->halt = true;
        __sync_bool_compare_and_swap(&proc->state, PROC_SUSPENDED, PROC_HALTED);
}
