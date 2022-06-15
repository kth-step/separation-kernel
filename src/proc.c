// See LICENSE file for copyright and license details.
#include "proc.h"

#include "cap.h"
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
void ProcReset(int pid) {
        /* Get the PCB */
        Proc *proc = &processes[pid];
        /* Set the process id to */
        proc->pid = pid;
        /* Set the process's kernel stack. */
        proc->ksp = &proc_stack[pid][STACK_SIZE / 8];
        for (int i = 0; i < STACK_SIZE / 8; i++)
                proc_stack[pid][i] = 0;
        /* Zero the capability table. */
        proc->cap_table = cap_tables[pid];
        for (int i = 0; i < N_CAPS; ++i) {
                CapNode *cap = &cap_tables[pid][i];
                if (!cap_is_deleted(cap)) {
                        CapRevoke(cap);
                        CapDelete(cap);
                }
        }
        proc->pc = 0;
        proc->listen_channel = -1;
        /* Set process to HALTED. */
        proc->state = PROC_HALTED;
}

void proc_init_memory(CapNode *pmp, CapNode *memory) {
        uint64_t begin = USER_MEMORY_BEGIN >> 12;
        uint64_t end = USER_MEMORY_END >> 12;
        uint64_t pmp_length = BOOT_PMP_LENGTH >> 12;
        uint64_t pmp_addr = begin | ((pmp_length - 1) >> 1);
        Cap cap_pmp = cap_mk_pmp(pmp_addr, 5);
        Cap cap_memory = cap_mk_memory(begin, end, begin, 7, 0);
        CapNode *sentinel = CapInitSentinel();
        ASSERT(CapInsert(cap_memory, memory, sentinel));
        ASSERT(CapInsert(cap_pmp, pmp, sentinel));
}

void proc_init_channels(CapNode *channel) {
        uint16_t begin = 0;
        uint16_t end = N_CHANNELS - 1;
        Cap cap = cap_mk_channels(begin, end, begin, 0);
        CapNode *sentinel = CapInitSentinel();
        ASSERT(CapInsert(cap, channel, sentinel));
}

void proc_init_time(CapNode time[N_CORES]) {
        for (int hartid = 0; hartid < N_CORES; hartid++) {
                CapNode *sentinel = CapInitSentinel();
                uint64_t begin = 0;
                uint64_t end = N_QUANTUM - 1;
                uint64_t free = 0;
                uint64_t id = 0;
                uint64_t id_free = 1;
                uint64_t id_end = 255;
                Cap cap = cap_mk_time(hartid, 0, begin, end, free, id, id_end, id_free);
                ASSERT(CapInsert(cap, &time[hartid], sentinel));
        }
}

void proc_init_supervisor(CapNode *cap_sup) {
        CapNode *sentinel = CapInitSentinel();
        Cap cap = cap_mk_supervisor(0, N_PROC, 0);
        ASSERT(CapInsert(cap, cap_sup, sentinel));
}

static void proc_init_boot_proc(Proc *boot) {
        CapNode *cap_table = boot->cap_table;
        proc_init_memory(&cap_table[0], &cap_table[1]);
        proc_init_channels(&cap_table[2]);
        proc_init_supervisor(&cap_table[3]);
        proc_init_time(&cap_table[4]);
        /* Set the initial PC. */
        // boot->pc = (uintptr_t)(pe_begin << 2);
        boot->pc = (uintptr_t)user_code;  // Temporary code.

        /* Set boot process to running. */
        boot->state = PROC_SUSPENDED;
}

/* Defined in proc.h */
void ProcInitProcesses(void) {
        /* Initialize processes. */
        for (int i = 0; i < N_PROC; i++)
                ProcReset(i);
        /*** Boot process ***/
        proc_init_boot_proc(&processes[0]);
}

void ProcHalt(Proc *proc) {
        proc->halt = true;
        __sync_bool_compare_and_swap(&proc->state, PROC_SUSPENDED, PROC_HALTED);
}
