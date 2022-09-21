// See LICENSE file for copyright and license details.
#include "proc.h"

#include "config.h"
#include "csr.h"
#include "kprint.h"

#define ARRAY_SIZE(x) ((sizeof(x) / sizeof(x[0])))

extern const uint64_t init_proc_start;

static inline void make_sentinel(cap_node_t* sentinel);
static cap_node_t* proc_init_memory(cap_node_t* cn);
static cap_node_t* proc_init_time(cap_node_t* cn);
static cap_node_t* proc_init_supervisor(cap_node_t* cn);
static cap_node_t* proc_init_channels(cap_node_t* cn);

/* Defined in proc.h */
proc_t processes[N_PROC];

void make_sentinel(cap_node_t* sentinel)
{
    sentinel->prev = sentinel;
    sentinel->next = sentinel;
    sentinel->cap = NULL_CAP;
}

cap_node_t* proc_init_memory(cap_node_t* cn)
{
    /* Node at beginning and end of capabiliy list */
    static cap_node_t sentinel;
    cap_t cap;

    make_sentinel(&sentinel);

    /* Make and insert pmp frame */
    cap = cap_mk_pmp(PMP);
    cap_node_insert(cap, cn++, &sentinel);

    uint64_t memory_slices[][2] = MEMORY_SLICES;
    for (int i = 0; i < ARRAY_SIZE(memory_slices); i++) {
        uint64_t begin = memory_slices[i][0];
        uint64_t end = memory_slices[i][1];
        cap = cap_mk_memory(begin, end, 7, begin, 0);
        cap_node_insert(cap, cn++, &sentinel);
    }

    return cn;
}

cap_node_t* proc_init_time(cap_node_t* cn)
{
    static cap_node_t sentinel;
    cap_t cap;

    make_sentinel(&sentinel);

    /* Default values of time slices */
    uint64_t begin = 0;
    uint64_t end = N_QUANTUM;
    uint64_t free = 0;

    for (int hartid = MIN_HARTID; hartid <= MAX_HARTID; hartid++) {
        cap = cap_mk_time(hartid, begin, end, free);
        cap_node_insert(cap, cn++, &sentinel);
    }
    return cn;
}

cap_node_t* proc_init_supervisor(cap_node_t* cn)
{
    static cap_node_t sentinel;
    cap_t cap;

    make_sentinel(&sentinel);

    cap = cap_mk_supervisor(0, N_PROC, 0);
    cap_node_insert(cap, cn++, &sentinel);

    return cn;
}

static cap_node_t* proc_init_channels(cap_node_t* cn)
{
    static cap_node_t sentinel;
    cap_t cap;

    make_sentinel(&sentinel);

    uint16_t begin = 0;
    uint16_t end = N_CHANNELS;

    cap = cap_mk_channels(begin, end, begin);
    cap_node_insert(cap, cn++, &sentinel);

    return cn;
}

/* Defined in proc.h */
void proc_init(void)
{
    /* Set the cap_table and process IDs */
    for (int i = 0; i < N_PROC; i++) {
        proc_t *proc = &processes[i];
        /* Set the process id */
        proc->pid = i;
        /* Capability table. */
        proc->cap_table = cap_tables[i];
        /* All processes are by default suspended */
        proc->state = PROC_STATE_SUSPENDED;
    }

    /* Initialize the init process */
    proc_t *init = &processes[0];
    cap_node_t* cn = init->cap_table;
    cn = proc_init_memory(cn);
    cn = proc_init_channels(cn);
    cn = proc_init_supervisor(cn);
    proc_init_time(cn);
    /* Set the initial PC. */
    init->regs.pc = init_proc_start;
    init->state = PROC_STATE_READY;
}

void proc_load_pmp(proc_t* proc)
{
    uint64_t pmpcfg = 0;
    uint64_t pmpaddr[8] = {0};
    for (int i = 0; i < 8; i++) {
        uint64_t pmpidx = proc->regs.pmp & 0xFF;
        if (pmpidx & 0x80)
            continue;
        cap_t cap = proc_get_cap(proc, i);
        if (cap_get_type(cap) != CAP_TYPE_PMP)
            continue;
        pmpcfg |= (cap_pmp_get_rwx(cap) << (i * 8)) | 0x18;
        pmpaddr[i] = (cap_pmp_get_addr(cap) << 10) | 0x3FF;
    }
    write_csr(pmpcfg0, pmpcfg);
    write_csr(pmpaddr0, pmpaddr[0]);
    write_csr(pmpaddr1, pmpaddr[1]);
    write_csr(pmpaddr2, pmpaddr[2]);
    write_csr(pmpaddr3, pmpaddr[3]);
    write_csr(pmpaddr4, pmpaddr[4]);
    write_csr(pmpaddr5, pmpaddr[5]);
    write_csr(pmpaddr6, pmpaddr[6]);
    write_csr(pmpaddr7, pmpaddr[7]);
}
