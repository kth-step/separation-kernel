// See LICENSE file for copyright and license details.
#include "proc.h"

#include "cap.h"
#include "cap_utils.h"
#include "config.h"
#include "stack.h"
#include "csr.h"

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
void ProcReset(int pid) {
        /* Get the PCB */
        Proc *proc = &processes[pid];
        /* Set the process id to */
        proc->pid = pid;
        /* Set the process's kernel stack. */
        proc->ksp = &proc_stack[pid][STACK_SIZE / 8];
        /* Zero the capability table. */
        proc->cap_table = cap_tables[pid];
        proc->pc = 0;
        proc->listen_channel = -1;
        /* Set process to HALTED. */
        proc->state = PROC_HALTED;
}

static void proc_init_memory(CapNode *pmp, CapNode *memory) {
        uint64_t begin = USER_MEMORY_BEGIN >> 12;
        uint64_t end = USER_MEMORY_END >> 12;
        uint64_t pmp_length = BOOT_PMP_LENGTH >> 12;
        uint64_t pmp_addr = begin | ((pmp_length - 1) >> 1);
        Cap cap_pmp = cap_mk_pmp(pmp_addr, 5);
        Cap cap_memory = cap_mk_memory(begin, end, 7, begin, 0);
        CapNode *sentinel = CapInitSentinel();
        CapInsert(cap_memory, memory, sentinel);
        CapInsert(cap_pmp, pmp, sentinel);
}

static void proc_init_channels(CapNode *channel) {
        uint16_t begin = 0;
        uint16_t end = N_CHANNELS - 1;
        Cap cap = cap_mk_channels(begin, end, begin, 0);
        CapNode *sentinel = CapInitSentinel();
        CapInsert(cap, channel, sentinel);
}

static void proc_init_time(CapNode time[N_CORES]) {
        for (int core = 0; core < N_CORES; core++) {
                CapNode *sentinel = CapInitSentinel();
                uint64_t pid = 0;
                uint64_t begin = 0;
                uint64_t end = N_QUANTUM;
                uint64_t free = 0;
                uint64_t depth = 0;
                Cap cap = cap_mk_time(core, pid, begin, end, free, depth);
                CapInsert(cap, &time[core], sentinel);
        }
}

static void proc_init_supervisor(CapNode *cap_sup) {
        CapNode *sentinel = CapInitSentinel();
        Cap cap = cap_mk_supervisor(0, N_PROC, 0);
        CapInsert(cap, cap_sup, sentinel);
}

static void proc_init_boot_proc(Proc *boot) {
        CapNode *cap_table = boot->cap_table;
        proc_init_memory(&cap_table[0], &cap_table[1]);
        proc_init_channels(&cap_table[2]);
        proc_init_supervisor(&cap_table[3]);
        proc_init_time(&cap_table[4]);
        /* Set the initial PC. */
        // boot->pc = (uintptr_t)(pe_begin << 2);
        boot->pc = (uint64_t)user_code;  // Temporary code.

        /* Set boot process to running. */
        boot->state = PROC_SUSPENDED;
}

/* Defined in proc.h */
void ProcInitProcesses(void) {
        /* Initialize processes. */
        uint64_t hartid = read_csr(mhartid);
        for (int i = 0; i < N_PROC; i++) {
                kprintf("Core %d: Init proc %lx\n", hartid, i);
                ProcReset(i);
        }
        /*** Boot process ***/
        kprintf("Core %d: Init boot proc 0\n", hartid);
        proc_init_boot_proc(&processes[0]);
}

void ProcHalt(Proc *proc) {
        proc->halt = true;
        __sync_bool_compare_and_swap(&proc->state, PROC_SUSPENDED, PROC_HALTED);
}
