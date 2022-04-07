// See LICENSE file for copyright and license details.
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "atomic.h"
#include "config.h"

typedef struct cap Cap;
typedef enum cap_type CapType;
typedef struct cap_memory_slice CapMemorySlice;
typedef struct cap_pmp_entry CapPmpEntry;
typedef struct cap_time_slice CapTimeSlice;
typedef struct cap_supervisor CapSupervisor;
typedef struct cap_endpoint CapEndpoint;

enum cap_type {
        CAP_INVALID,
        CAP_PMP_ENTRY,
        CAP_MEMORY_SLICE,
        CAP_TIME_SLICE,
        CAP_ENDPOINT,
        CAP_SUPERVISOR
};

struct cap_memory_slice {
        uint8_t perm;
        uint64_t begin;
        uint64_t end;
};

struct cap_pmp_entry {
        uint8_t cfg;
        uint64_t addr;
        uint64_t addr_tor;
};

struct cap_time_slice {
        uint8_t hartid;
        uint8_t tsid;
        uint8_t fuel;
        uint16_t begin;
        uint16_t end;
};

struct cap_supervisor {
        uint8_t pid;
};

struct cap_endpoint {
        /* TODO: */
};

struct cap {
        Cap *prev;
        Cap *next;
        uint64_t field0;
        uint64_t field1;
};

_Static_assert(sizeof(Cap) == 32, "Capability node size error");

static inline CapType cap_get_type(const Cap *cap) {
        return cap->field0 >> 56;
}

/* Check capability type */
static inline bool cap_is_type(const Cap *cap, CapType t) {
        return cap_get_type(cap) == t;
}

static inline bool cap_is_deleted(Cap *node) {
        return is_marked(node->prev) && node->next == NULL;
}

static inline void cap_set_memory_slice(Cap *cap, const CapMemorySlice ms) {
        cap->field0 = (uint64_t)CAP_MEMORY_SLICE << 56;
        cap->field0 |= ms.begin;
        cap->field1 = (uint64_t)ms.perm << 56;
        cap->field1 |= ms.end;
}

static inline CapMemorySlice cap_get_memory_slice(const Cap *cap) {
        CapMemorySlice ms;
        ms.begin = cap->field0 & 0xFFFFFFFFFFFFFFUL;
        ms.perm = cap->field1 >> 56;
        ms.end = cap->field1 & 0xFFFFFFFFFFFFFFUL;
        return ms;
}

static inline void cap_set_pmp_entry(Cap *cap, const CapPmpEntry pe) {
        cap->field0 = (uint64_t)CAP_PMP_ENTRY << 56;
        cap->field0 |= pe.addr;
        cap->field1 = (uint64_t)pe.cfg << 56;
        cap->field1 |= pe.addr_tor;
}

static inline CapPmpEntry cap_get_pmp_entry(const Cap *cap) {
        CapPmpEntry pe;
        pe.addr = cap->field0 & 0xFFFFFFFFFFFFFFUL;
        pe.cfg = cap->field1 >> 56;
        pe.addr_tor = cap->field1 & 0xFFFFFFFFFFFFFFUL;
        return pe;
}

static inline void cap_set_time_slice(Cap *cap, const CapTimeSlice ts) {
        cap->field0 = (uint64_t)CAP_TIME_SLICE << 56;
        cap->field0 |= (uint64_t)ts.hartid << 48;
        cap->field0 |= (uint64_t)ts.tsid << 40;
        cap->field0 |= (uint64_t)ts.fuel << 32;
        cap->field0 |= (ts.begin << 16);
        cap->field0 |= ts.end;
        cap->field1 = 0;
}

static inline CapTimeSlice cap_get_time_slice(Cap *cap) {
        CapTimeSlice ts;
        ts.hartid = (cap->field0 >> 48) & 0xFF;
        ts.tsid = (cap->field0 >> 40) & 0xFF;
        ts.fuel = (cap->field0 >> 32) & 0xFF;
        ts.begin = (cap->field0 >> 16) & 0xFFFF;
        ts.end = cap->field0 & 0xFFFF;
        return ts;
}

static inline void cap_set_supervisor(Cap *cap, const CapSupervisor ts) {
        cap->field0 = (uint64_t)CAP_TIME_SLICE << 56;
        cap->field0 |= ts.pid;
        cap->field1 = 0;
}

static inline CapSupervisor cap_get_supervisor(Cap *cap) {
        CapSupervisor sup;
        sup.pid = (cap->field0 >> 48) & 0xFF;
        return sup;
}

/* Check if a PmpEntry is child of a Memory Slice */
static inline bool cap_is_child_ts_ts(const CapTimeSlice parent,
                                      const CapTimeSlice child) {
        return (parent.hartid == child.hartid) && (parent.tsid < child.tsid) &&
               (child.tsid <= parent.fuel);
}

/* Assumes same hartid */
/* Returns true if the time slices intersect */
static inline bool cap_is_intersect_ts_ts(const CapTimeSlice ts0,
                                          const CapTimeSlice ts1) {
        return !(ts0.fuel < ts1.tsid || ts1.fuel < ts0.tsid);
}

static inline bool cap_is_intersect_ms_ms(const CapMemorySlice ms0,
                                          const CapMemorySlice ms1) {
        return !(ms0.end < ms1.begin || ms1.end < ms0.begin);
}

/* Get the beginning and end of a PMP entry. */
static inline void cap_bound_of_pe(const CapPmpEntry pe, uintptr_t *begin,
                                   uintptr_t *end) {
        int A = (pe.cfg >> 3) & 0x3;
        switch (A) {
                case 1: /* TOR */
                        *begin = pe.addr;
                        *end = pe.addr_tor;
                        break;
                case 2: /* NA4 */
                        *begin = pe.addr;
                        *end = pe.addr + 4;
                        break;
                case 3: /* NAPOT */
                        /* Bit magic! */
                        *begin = (pe.addr & (pe.addr + 1));
                        *end = *begin + (pe.addr ^ (pe.addr + 1)) + 1;
                        break;
                default: /* NONE, should never happen. */
                        __builtin_unreachable();
                        break;
        }
}

static inline bool cap_is_intersect_ms_pe(const CapMemorySlice ms,
                                          const CapPmpEntry pe) {
        uintptr_t begin, end;
        cap_bound_of_pe(pe, &begin, &end);
        return !(ms.end < begin || end < ms.begin);
}

static inline bool cap_is_intersect_pe_pe(const CapPmpEntry pe0,
                                          const CapPmpEntry pe1) {
        uintptr_t begin0, end0;
        uintptr_t begin1, end1;
        cap_bound_of_pe(pe0, &begin0, &end0);
        cap_bound_of_pe(pe1, &begin1, &end1);
        return !(end0 < begin1 || end1 < begin0);
}

/* Assumes child of existing capability. */
static inline bool cap_validate_ts(const CapTimeSlice ts) {
        return ts.begin < ts.end;
}

/* Assumes child of existing capability. */
static inline bool cap_validate_ms(const CapMemorySlice ms) {
        return ms.begin < ms.end;
}

/* Assumes child of existing capability. */
static inline bool cap_validate_pe(const CapPmpEntry pe) {
        uintptr_t begin, end;
        cap_bound_of_pe(pe, &begin, &end);
        uintptr_t a = (pe.cfg >> 3) & 0x3;
        uintptr_t rwx = pe.cfg & 0x7;
        /* TODO: check for illegal rwx values */
        return begin < end && (a > 0) && (rwx > 0);
}

static inline bool cap_is_valid(Cap *cap) {
        switch (cap_get_type(cap)) {
                case CAP_TIME_SLICE:
                        return cap_validate_ts(cap_get_time_slice(cap));
                case CAP_MEMORY_SLICE:
                        return cap_validate_ms(cap_get_memory_slice(cap));
                case CAP_PMP_ENTRY:
                        return cap_validate_pe(cap_get_pmp_entry(cap));
                default:
                        return true;
        }
}

/* Check if a PmpEntry is child of a Memory Slice */
static inline bool cap_is_child_ms_pe(const CapMemorySlice parent,
                                      const CapPmpEntry child) {
        uintptr_t pe_begin, pe_end;
        cap_bound_of_pe(child, &pe_begin, &pe_end);
        return parent.begin <= pe_begin && pe_end <= parent.end &&
               ((child.cfg & 0x7) == (parent.perm & child.cfg));
}

/* Check if a Memory Slice is child of a Memory Slice */
static inline bool cap_is_child_ms_ms(const CapMemorySlice parent,
                                      const CapMemorySlice child) {
        return parent.begin <= child.begin && child.end <= parent.end;
}

static inline bool cap_is_child_ms(CapMemorySlice parent, Cap *child) {
        if (cap_is_type(child, CAP_MEMORY_SLICE))
                return cap_is_child_ms_ms(parent, cap_get_memory_slice(child));
        return cap_is_child_ms_pe(parent, cap_get_pmp_entry(child));
}

static inline bool cap_is_child(Cap *parent, Cap *child) {
        if (cap_is_type(parent, CAP_MEMORY_SLICE))
                return cap_is_child_ms(cap_get_memory_slice(parent), child);
        if (cap_is_type(parent, CAP_TIME_SLICE) &&
            cap_is_type(parent, CAP_TIME_SLICE))
                return cap_is_child_ts_ts(cap_get_time_slice(parent),
                                          cap_get_time_slice(child));
        return false;
}

void CapRevoke(Cap *cap);
bool CapDelete(Cap *cap);
bool CapMove(Cap *dest, Cap *src);
bool CapInsert(Cap *parent, Cap *child);
