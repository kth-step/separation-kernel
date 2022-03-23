// See LICENSE file for copyright and license details.
#include "capabilities.h"

static inline int cap_is_type(Capability *c, CapType t) {
        return c->type == t;
}

static inline int CapIsChildTsTs(TimeSlice *parent, TimeSlice *child) {
        return (parent->cid == child->cid) && (parent->tsid < child->tsid) &&
               (child->tsid < parent->tsid + parent->fuel);
}

static inline int bound_of_pe(PmpEntry *pe, uintptr_t *begin, uintptr_t *end) {
        int A = (pe->cfg >> 3) & 0x3;
        switch (A) {
                case 1:
                        *begin = pe->addr;
                        *end = pe->addr_tor;
                        break;
                case 2:
                        *begin = pe->addr;
                        *end = pe->addr + 4;
                        break;
                case 3:
                        *begin = (pe->addr & (pe->addr + 1));
                        *end = *begin + (pe->addr ^ (pe->addr + 1)) + 1;
                        break;
                default:
                        break;
        }
        return A;
}

static inline int is_child_ms_pe(MemorySlice *parent, PmpEntry *child) {
        uintptr_t pe_begin, pe_end;
        bound_of_pe(child, &pe_begin, &pe_end);
        return parent->begin <= pe_begin && pe_end <= parent->end;
}

static inline int is_child_ms_ms(MemorySlice *parent, MemorySlice *child) {
        return parent->begin <= child->begin && child->end <= parent->end;
}

static inline int CapIsChild(Capability *parent, Capability *child) {
        if (cap_is_type(parent, CAP_TIME_SLICE) &&
            cap_is_type(child, CAP_TIME_SLICE)) {
                return CapIsChildTsTs(&parent->time_slice, &child->time_slice);
        } else if (cap_is_type(parent, CAP_MEMORY_SLICE) &&
                   cap_is_type(child, CAP_PMP_ENTRY)) {
                return is_child_ms_pe(&parent->memory_slice, &child->pmp_entry);
        } else if (cap_is_type(parent, CAP_MEMORY_SLICE) &&
                   cap_is_type(child, CAP_MEMORY_SLICE)) {
                return is_child_ms_ms(&parent->memory_slice, &child->memory_slice);
        }
        return 0;
}
