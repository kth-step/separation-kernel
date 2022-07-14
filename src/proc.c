// See LICENSE file for copyright and license details.
#include "proc.h"

#include "config.h"
#include "csr.h"
#include "kprint.h"

/* Temporary. */
extern void user_code();

/* Defined in proc.h */
struct proc processes[N_PROC];

#define MAKE_SENTINEL(s)          \
        do {                      \
                s.prev = &s;      \
                s.next = &s;      \
                s.cap = NULL_CAP; \
        } while (0)

static struct cap_node* proc_init_memory(struct cap_node* cn)
{
        /* Node at beginning and end of capabiliy list */
        static struct cap_node sentinel;
        struct cap cap;

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

static struct cap_node* proc_init_time(struct cap_node* cn)
{
        static struct cap_node sentinel;
        struct cap cap;

        MAKE_SENTINEL(sentinel);

        /* Default values of time slices */
        uint64_t pid = 0;
        uint64_t begin = 0;
        uint64_t end = N_QUANTUM;
        uint64_t free = 0;
        uint64_t depth = 0;

        for (int hartid = MIN_HARTID; hartid <= MAX_HARTID; hartid++) {
                cap = cap_mk_time(hartid, pid, begin, end, free, depth);
                cap_node_insert(cap, cn++, &sentinel);
        }
        return cn;
}

static struct cap_node* proc_init_supervisor(struct cap_node* cn)
{
        static struct cap_node sentinel;
        struct cap cap;

        MAKE_SENTINEL(sentinel);

        cap = cap_mk_supervisor(0, N_PROC, 0);
        cap_node_insert(cap, cn++, &sentinel);

        return cn;
}

static struct cap_node* proc_init_channels(struct cap_node* cn)
{
        static struct cap_node sentinel;
        struct cap cap;

        MAKE_SENTINEL(sentinel);

        uint16_t begin = 0;
        uint16_t end = N_CHANNELS;

        cap = cap_mk_channels(begin, end, begin, 0);
        cap_node_insert(cap, cn++, &sentinel);

        return cn;
}

static void __proc_init_boot(struct proc* boot)
{
        struct cap_node* cn;
        cn = boot->cap_table;
        cn = proc_init_memory(cn);
        cn = proc_init_channels(cn);
        cn = proc_init_supervisor(cn);
        cn = proc_init_time(cn);
        /* Set the initial PC. */
        // boot->pc = (uintptr_t)(pe_begin << 2);
        boot->regs.pc = (uint64_t)user_code; // Temporary code.
        boot->state = PROC_STATE_READY;
}

/* Initializes one process. */
static void __proc_init(struct proc* proc, int pid)
{
        /* Set the process id */
        proc->pid = pid;
        /* Capability table. */
        proc->cap_table = cap_tables[pid];
        /* All processes are by default suspended */
        proc->state = PROC_STATE_SUSPENDED;
        /* channel == -1 means not subscribed to any channel */
        proc->channel = -1;
}

/* Defined in proc.h */
void proc_init(void)
{
        for (int i = 0; i < N_PROC; i++)
                __proc_init(&processes[i], i);
        __proc_init_boot(&processes[0]);
}
