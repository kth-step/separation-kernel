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
        /* Set the capability table. */
        proc->cap_table = cap_tables[pid];
        for (int i = 0; i < N_CAPS; ++i) {
                proc->cap_table[i].next = NULL;
                proc->cap_table[i].prev = (Capability *)mark_bit;
        }
        /* Set process to HALTED. */
        proc->state = PROC_HALTED;
}

static void proc_init_boot_proc(void) {
        /* Set the initial PC. */
        processes[0].pc = BOOT_PC;
        /* Set it to running. */
        processes[0].state = PROC_RUNNING;

        /* Gives all time capabilities to boot process */
        for (int i = 0; i < N_PROC; ++i) {
                CapTimeSlice ts;
                ts.hartid = i;
                ts.tsid = 0;
                ts.begin = 0;
                ts.end = N_QUANTUM;
                ts.fuel = 255;
                cap_set_time_slice(&processes[0].cap_table[i + 2], ts);
                processes[0].cap_table[i + 2].next = NULL;
                processes[0].cap_table[i + 2].prev = NULL;
        }
        CapMemorySlice ms;
        ms.perm = 0x7;
        ms.begin = USER_MEMORY_BEGIN;
        ms.end = USER_MEMORY_END;
        cap_set_memory_slice(&processes[0].cap_table[1], ms);
        CapPmpEntry pe;
        pe.cfg = 0x5 | (3 << 3);
        pe.addr = USER_MEMORY_BEGIN | (0x1000 - 1);
        pe.addr_tor = 0;
        cap_set_pmp_entry(&processes[0].cap_table[0], pe);
        processes[0].cap_table[0].prev = &processes[0].cap_table[1];
        processes[0].cap_table[0].next = NULL;
        processes[0].cap_table[1].next = &processes[0].cap_table[0];
        processes[0].cap_table[1].prev = NULL;

        /* TODO: Set pc to beginning of CapPmpEntry/CapMemorySlice. */
}

/* Defined in proc.h */
void ProcInitProcesses(void) {
        /* Initialize processes. */
        for (int i = 0; i < N_PROC; i++)
                proc_init_proc(i);
        /*** Boot process ***/
        proc_init_boot_proc();
}
