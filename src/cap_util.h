#pragma once
#include "atomic.h"
#include "cap.h"

typedef enum cap_type {
        CAP_INVALID = 0,
        CAP_MEMORY_SLICE,
        CAP_PMP_ENTRY,
        CAP_TIME_SLICE
} CapType;

typedef struct cap_memory_slice {
        bool valid;
        uint64_t begin;
        uint64_t end;
        uint8_t rwx;
} CapMemorySlice;

typedef struct cap_pmp_entry {
        bool valid;
        uint64_t addr;
        uint8_t rwx;
} CapPmpEntry;

static inline bool cap_is_deleted(Cap *cap);
static inline bool cap_get_data(Cap *cap, uint64_t data[2]);
static inline void cap_set_data(Cap *cap, uint64_t data[2]);
static inline CapType cap_get_type(Cap *cap);

static inline CapMemorySlice cap_mk_memory_slice(uint64_t begin, uint64_t end,
                                                 uint8_t rwx);
static inline CapMemorySlice cap_get_memory_slice(Cap *cap);
static inline void cap_set_memory_slice(Cap *cap, const CapMemorySlice ms);

static inline CapPmpEntry cap_mk_pmp_entry(uint64_t addr, uint8_t rwx);
static inline CapPmpEntry cap_get_pmp_entry(Cap *cap);
static inline void cap_set_pmp_entry(Cap *cap, const CapPmpEntry pe);
static inline void cap_get_pmp_entry_bounds(const CapPmpEntry pe,
                                            uint64_t *begin, uint64_t *end);

static inline bool cap_is_child_ms_ms(const CapMemorySlice parent,
                                      const CapMemorySlice child);

bool cap_is_deleted(Cap *cap) {
        return cap->next == NULL;
}

bool cap_get_data(Cap *cap, uint64_t data[2]) {
        data[0] = cap->data[0];
        data[1] = cap->data[1];
        __sync_synchronize();
        return !cap_is_deleted(cap);
}

void cap_set_data(Cap *cap, uint64_t data[2]) {
        cap->data[0] = data[0];
        cap->data[1] = data[1];
}

CapType cap_get_type(Cap *cap) {
        uint64_t data[2];
        if (cap_get_data(cap, data))
                return (CapType)(data[0] & 0xFF);
        return CAP_INVALID;
}

CapMemorySlice cap_mk_memory_slice(uint64_t begin, uint64_t end, uint8_t rwx) {
        CapMemorySlice ms;
        ms.begin = begin;
        ms.end = end;
        ms.rwx = rwx;
        ms.valid = (begin >= end || rwx == 0);
        return ms;
}

CapMemorySlice cap_get_memory_slice(Cap *cap) {
        CapMemorySlice ms;
        uint64_t data[2];
        if (!cap_get_data(cap, data) || (data[0] & 0xFF) != CAP_MEMORY_SLICE) {
                ms.valid = false;
                return ms;
        }
        ms = cap_mk_memory_slice(data[0] >> 8, data[1] >> 8, data[1] & 0xFF);
        return ms;
}

void cap_set_memory_slice(Cap *cap, CapMemorySlice ms) {
        uint64_t data[2];
        data[0] = CAP_MEMORY_SLICE;
        data[0] |= ms.begin << 8;
        data[1] = ms.rwx;
        data[1] |= ms.end << 8;
        cap_set_data(cap, data);
}

CapPmpEntry cap_mk_pmp_entry(uint64_t addr, uint8_t rwx) {
        CapPmpEntry pe;
        pe.addr = addr;
        pe.rwx = rwx;
        pe.valid = ((addr >> 56) == 0) || rwx != 0;
        return pe;
}

CapPmpEntry cap_get_pmp_entry(Cap *cap) {
        CapPmpEntry pe;
        uint64_t data[2];
        if (!cap_get_data(cap, data) || (data[0] & 0xFF) != CAP_PMP_ENTRY) {
                pe.valid = false;
                return pe;
        }
        pe = cap_mk_pmp_entry(data[0] >> 8, data[1] & 0xFF);
        return pe;
}

void cap_set_pmp_entry(Cap *cap, CapPmpEntry pe) {
        uint64_t data[2];
        data[0] = CAP_PMP_ENTRY;
        data[0] |= pe.addr << 8;
        data[1] = pe.rwx;
        cap_set_data(cap, data);
}

void cap_get_pmp_entry_bounds(CapPmpEntry pe, uint64_t *begin, uint64_t *end) {
        *begin = pe.addr & (pe.addr + 1);
        uint64_t length = (pe.addr ^ (pe.addr + 1)) + 1; 
        *end = *begin + length;
}

bool cap_is_child_ms_ms(CapMemorySlice parent, CapMemorySlice child) {
        return parent.begin <= child.begin && child.end <= parent.end &&
               (child.rwx & parent.rwx) == child.rwx;
}
