// See LICENSE file for copyright and license details.
#include "proc.h"

#include "cap.h"
#include "cap_utils.h"
#include "config.h"
#include "csr.h"
#include "kprint.h"

/* Temporary. */
extern void user_code();

/* Defined in proc.h */
struct proc processes[N_PROC];


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
        uint16_t end = N_CHANNELS;
        Cap cap = cap_mk_channels(begin, end, begin, 0);
        CapNode *sentinel = CapInitSentinel();
        CapInsert(cap, channel, sentinel);
}

static void proc_init_time(CapNode time[N_CORES]) {
        for (int hartid = MIN_HARTID; hartid <= MAX_HARTID; hartid++) {
                CapNode *sentinel = CapInitSentinel();
                uint64_t pid = 0;
                uint64_t begin = 0;
                uint64_t end = N_QUANTUM;
                uint64_t free = 0;
                uint64_t depth = 0;
                Cap cap = cap_mk_time(hartid, pid, begin, end, free, depth);
                CapInsert(cap, &time[hartid - MIN_HARTID], sentinel);
        }
}

static void proc_init_supervisor(CapNode *cap_sup) {
        CapNode *sentinel = CapInitSentinel();
        Cap cap = cap_mk_supervisor(0, N_PROC, 0);
        CapInsert(cap, cap_sup, sentinel);
}

static void __proc_init_boot(struct proc *boot) {
        CapNode *cap_table = boot->cap_table;
        proc_init_memory(&cap_table[0], &cap_table[1]);
        proc_init_channels(&cap_table[2]);
        proc_init_supervisor(&cap_table[3]);
        proc_init_time(&cap_table[4]);
        /* Set the initial PC. */
        // boot->pc = (uintptr_t)(pe_begin << 2);
        boot->regs.pc = (uint64_t)user_code;  // Temporary code.
        boot->state = PS_READY;
}

/* Initializes one process. */
static void __proc_init(struct proc *proc, int pid) {
        /* Set the process id */
        proc->pid = pid;
        /* Capability table. */
        proc->cap_table = cap_tables[pid];
        /* All processes are by default suspended */
        proc->state = PS_SUSPENDED;
        /* channel == -1 means not subscribed to any channel */
        proc->channel = -1;
}

/* Defined in proc.h */
void proc_init(void) {
        for (int i = 0; i < N_PROC; i++)
                __proc_init(&processes[i], i);
        __proc_init_boot(&processes[0]);
}
