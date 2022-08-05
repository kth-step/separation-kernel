// See LICENSE file for copyright and license details.
#include "proc.h"

#include "config.h"
#include "csr.h"
#include "kprint.h"

static cap_node_t* proc_init_memory(cap_node_t* cn);
static cap_node_t* proc_init_time(cap_node_t* cn);
static cap_node_t* proc_init_supervisor(cap_node_t* cn);
static cap_node_t* proc_init_channels(cap_node_t* cn);
static void proc_init_boot(proc_t* boot);
static void proc_init_proc(proc_t* proc, int pid);

/* Temporary. */
extern void user_code();

/* Defined in proc.h */
proc_t processes[N_PROC];

#define MAKE_SENTINEL(s)          \
        do {                      \
                s.prev = &s;      \
                s.next = &s;      \
                s.cap = NULL_CAP; \
        } while (0)

cap_node_t* proc_init_memory(cap_node_t* cn)
{
        /* Node at beginning and end of capabiliy list */
        static cap_node_t sentinel;
        cap_t cap;

        MAKE_SENTINEL(sentinel);

        /* Arguments for cap_mk_... */
        uint64_t begin = USER_MEMORY_BEGIN >> 12;
        uint64_t end = USER_MEMORY_END >> 12;

        uint64_t pmp_length = BOOT_PMP_LENGTH >> 12;
        uint64_t pmp_addr = begin | ((pmp_length - 1) >> 1);

        /* Make and insert pmp frame */
        cap = cap_mk_pmp(pmp_addr, 5);
        cap_node_insert(cap, cn++, &sentinel);

        /* Make and insert memory slice */
        cap = cap_mk_memory(begin, end, 7, begin, 0);
        cap_node_insert(cap, cn++, &sentinel);

        return cn;
}

cap_node_t* proc_init_time(cap_node_t* cn)
{
        static cap_node_t sentinel;
        cap_t cap;

        MAKE_SENTINEL(sentinel);

        /* Default values of time slices */
        uint64_t begin = 0;
        uint64_t end = N_QUANTUM;
        uint64_t free = 0;
        uint64_t depth = 0;

        for (int hartid = MIN_HARTID; hartid <= MAX_HARTID; hartid++) {
                cap = cap_mk_time(hartid, begin, end, free, depth);
                cap_node_insert(cap, cn++, &sentinel);
        }
        return cn;
}

cap_node_t* proc_init_supervisor(cap_node_t* cn)
{
        static cap_node_t sentinel;
        cap_t cap;

        MAKE_SENTINEL(sentinel);

        cap = cap_mk_supervisor(0, N_PROC, 0);
        cap_node_insert(cap, cn++, &sentinel);

        return cn;
}

static cap_node_t* proc_init_channels(cap_node_t* cn)
{
        static cap_node_t sentinel;
        cap_t cap;

        MAKE_SENTINEL(sentinel);

        uint16_t begin = 0;
        uint16_t end = N_CHANNELS;

        cap = cap_mk_channels(begin, end, begin);
        cap_node_insert(cap, cn++, &sentinel);

        return cn;
}

void proc_init_boot(proc_t* boot)
{
        cap_node_t* cn = boot->cap_table;
        cn = proc_init_memory(cn);
        cn = proc_init_channels(cn);
        cn = proc_init_supervisor(cn);
        proc_init_time(cn);
        /* Set the initial PC. */
        // boot->pc = (uintptr_t)(pe_begin << 2);
        boot->regs.pc = (uint64_t)user_code;  // Temporary code.
        boot->state = PROC_STATE_READY;
}

/* Initializes one process. */
void proc_init_proc(proc_t* proc, int pid)
{
        /* Set the process id */
        proc->pid = pid;
        /* Capability table. */
        proc->cap_table = cap_tables[pid];
        /* All processes are by default suspended */
        proc->state = PROC_STATE_SUSPENDED;
}

/* Defined in proc.h */
void proc_init(void)
{
        for (int i = 0; i < N_PROC; i++)
                proc_init_proc(&processes[i], i);
        proc_init_boot(&processes[0]);
}
