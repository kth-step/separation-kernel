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

void proc_init_memory(Cap *pmp, Cap *memory) {
        uint64_t begin = USER_MEMORY_BEGIN;
        uint64_t end = USER_MEMORY_END;
        uint64_t pmp_length = BOOT_PMP_LENGTH;
        uint64_t pmp_addr = begin | ((pmp_length -1) >> 1);
        CapPmpEntry pe = cap_mk_pmp_entry(pmp_addr, CAP_RX);
        CapMemorySlice ms = cap_mk_memory_slice(begin, end, CAP_RWX);
        cap_set_pmp_entry(pmp, pe);
        cap_set_memory_slice(memory, ms);
        Cap *sentinel = CapInitSentinel(0);
        CapAppend(memory, sentinel);
        CapAppend(pmp, sentinel);
}

void proc_init_channels(Cap *channel) {
}

void proc_init_time(Cap time[N_PROC]) {
        for (int i = 0; i < N_PROC; i++) {
                Cap *sentinel = CapInitSentinel(1 + i);
                uint8_t begin = 0;
                uint8_t end = N_QUANTUM;
                uint8_t tsid = 0;
                uint8_t fuel = 255;
                CapTimeSlice ts = cap_mk_time_slice(i, begin, end, tsid, fuel);
                cap_set_time_slice(&time[i], ts);
                CapAppend(&time[i], sentinel);
        }
}



static void proc_init_boot_proc(Proc *boot) {
        Cap *cap_table = boot->cap_table;
        proc_init_memory(&cap_table[0], &cap_table[1]);
        proc_init_channels(&cap_table[2]);
        proc_init_time(&cap_table[3]);
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
