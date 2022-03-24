// See LICENSE file for copyright and license details.
#include "capabilities.h"

/* Check capability type */
static inline int is_type(Capability *c, CapType t) {
        return c->type == t;
}

/* Check if a PmpEntry is child of a Memory Slice */
static inline int is_child_ts_ts(TimeSlice *parent, TimeSlice *child) {
        return (parent->cid == child->cid) && (parent->tsid < child->tsid) &&
               (child->tsid < parent->tsid + parent->fuel);
}

/* Get the beginning and end of a PMP entry. */
static inline void bound_of_pe(PmpEntry *pe, uintptr_t *begin, uintptr_t *end) {
        int A = (pe->cfg >> 3) & 0x3;
        switch (A) {
                case 1: /* TOR */
                        *begin = pe->addr;
                        *end = pe->addr_tor;
                        break;
                case 2: /* NA4 */
                        *begin = pe->addr;
                        *end = pe->addr + 4;
                        break;
                case 3: /* NAPOT */
                        /* Bit magic! */
                        *begin = (pe->addr & (pe->addr + 1));
                        *end = *begin + (pe->addr ^ (pe->addr + 1)) + 1;
                        break;
                default: /* NONE, should never happen. */
                        /* TODO: throw some error */
                        break;
        }
}

/* Check if a PmpEntry is child of a Memory Slice */
static inline int is_child_ms_pe(MemorySlice *parent, PmpEntry *child) {
        uintptr_t pe_begin, pe_end;
        bound_of_pe(child, &pe_begin, &pe_end);
        return parent->begin <= pe_begin && pe_end <= parent->end;
}

/* Check if a Memory Slice is child of a Memory Slice */
static inline int is_child_ms_ms(MemorySlice *parent, MemorySlice *child) {
        return parent->begin <= child->begin && child->end <= parent->end;
}

/* Check relation between two capabilities */
static inline int is_child(Capability *parent, Capability *child) {
        if (is_type(parent, CAP_TIME_SLICE) && is_type(child, CAP_TIME_SLICE)) {
                /* Time slice child of time slice? */
                return is_child_ts_ts(&parent->time_slice, &child->time_slice);
        } else if (is_type(parent, CAP_MEMORY_SLICE) && is_type(child, CAP_PMP_ENTRY)) {
                /* PMP Entry child of memory slice? */
                return is_child_ms_pe(&parent->memory_slice, &child->pmp_entry);
        } else if (is_type(parent, CAP_MEMORY_SLICE) && is_type(child, CAP_MEMORY_SLICE)) {
                /* Memory slice child of memory slice? */
                return is_child_ms_ms(&parent->memory_slice, &child->memory_slice);
        }
        /* If no case match, return false */
        return 0;
}
