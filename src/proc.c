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
#define INIT_STACK_OFFSET (STACK_SIZE / 8 - 32)

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
        proc->ksp = &proc_stack[pid][INIT_STACK_OFFSET];
        for (int i = INIT_STACK_OFFSET; i < STACK_SIZE/8; i++)
               proc_stack[pid][i] = 0; 
        /* Set the capability table. */
        proc->cap_table = cap_tables[pid];
        for (int i = 0; i < N_CAPS; ++i) {
                CapRevoke(&cap_tables[pid][i]);
                CapDelete(&cap_tables[pid][i]);
        }
        proc->epid = -1;
        /* Set process to HALTED. */
        proc->state = PROC_HALTED;
}

void proc_init_memory(Cap *pmp, Cap *memory) {
        uint64_t begin = USER_MEMORY_BEGIN;
        uint64_t end = USER_MEMORY_END;
        uint64_t pmp_length = BOOT_PMP_LENGTH;
        uint64_t pmp_addr = begin | ((pmp_length - 1) >> 1);
        CapPmpEntry pe = cap_mk_pmp_entry(pmp_addr, CAP_RX);
        CapMemorySlice ms = cap_mk_memory_slice(begin, end, CAP_RWX);
        cap_set_pmp_entry(pmp, pe);
        cap_set_memory_slice(memory, ms);
        Cap *sentinel = CapInitSentinel();
        CapAppend(memory, sentinel);
        CapAppend(pmp, sentinel);
}

void proc_init_channels(Cap *channel) {
        uint16_t begin = 0;
        uint16_t end = N_CHANNELS - 1;
        CapChannels ch = cap_mk_channels(begin, end);
        cap_set_channels(channel, ch);
        Cap *sentinel = CapInitSentinel();
        CapAppend(channel, sentinel);
}

void proc_init_time(Cap time[N_CORES]) {
        for (int i = 0; i < N_CORES; i++) {
                Cap *sentinel = CapInitSentinel();
                uint16_t begin = 0;
                uint16_t end = N_QUANTUM - 1;
                uint8_t tsid = 0;
                uint8_t fuel = 255;
                CapTimeSlice ts = cap_mk_time_slice(i, begin, end, tsid, fuel);
                cap_set_time_slice(&time[i], ts);
                CapAppend(&time[i], sentinel);
        }
}

void proc_init_supervisor(Cap cap_sups[N_PROC]) {
        Cap *sentinel = CapInitSentinel();
        for (int i = 0; i < N_PROC; i++) {
                CapSupervisor sup = cap_mk_supervisor(i);
                cap_set_supervisor(&cap_sups[i], sup);
                CapAppend(&cap_sups[i], sentinel);
        }
}

static void proc_init_boot_proc(Proc *boot) {
        Cap *cap_table = boot->cap_table;
        proc_init_memory(&cap_table[0], &cap_table[1]);
        proc_init_channels(&cap_table[2]);
        proc_init_time(&cap_table[3]);
        proc_init_supervisor(&cap_table[3 + N_CORES]);
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

bool ProcReset(int8_t pid, Cap *cap_pmp) {
        if (pid < 0 || pid >= N_PROC) {
                return false;
        }
        Proc *proc = &processes[pid];
        if (proc->state != PROC_HALTED)
                return false;
        CapPmpEntry pmp = cap_get_pmp_entry(cap_pmp);
        if (!pmp.valid)
                return false;
        proc_init_proc(pid);
        uint64_t begin, end;
        cap_get_pmp_entry_bounds(pmp, &begin, &end);
        bool pmp_done = CapMove(&proc->cap_table[0], cap_pmp) &&
                        ProcPmpLoad(pid, 0, pmp.addr, pmp.rwx);
        if (pmp_done) {
                *proc->ksp = (uintptr_t)user_code;  // Temporary code.
        }
        return pmp_done;
}

bool ProcPmpLoad(int8_t pid, uint8_t index, uint64_t addr, uint8_t rwx) {
        if (index >= 8 || pid >= N_PROC || pid < 0)
                return false;
        Proc *proc = &processes[pid];
        uint64_t cfg = 0x18 | rwx;
        uint64_t mask = 0xFF << (index * 8);
        while (1) {
                uint64_t pmpcfg = proc->pmpcfg;
                uint64_t new_pmpcfg = pmpcfg | (cfg << (index * 8));
                if (pmpcfg & mask)
                        return false;
                proc->pmpaddr[index] = addr;
                if (__sync_bool_compare_and_swap(&proc->pmpcfg, pmpcfg,
                                                 new_pmpcfg))
                        return true;
        }
}

bool ProcPmpUnload(int8_t pid, uint8_t index) {
        if (index >= 8 || pid >= N_PROC || pid < 0)
                return false;
        Proc *proc = &processes[pid];
        uint64_t mask = 0xFF << (index * 8);
        while (1) {
                uint64_t pmpcfg = proc->pmpcfg;
                uint64_t new_pmpcfg = pmpcfg & ~mask;
                if (pmpcfg & mask)
                        return false;
                if (__sync_bool_compare_and_swap(&proc->pmpcfg, pmpcfg,
                                                 new_pmpcfg))
                        return true;
        }
}
