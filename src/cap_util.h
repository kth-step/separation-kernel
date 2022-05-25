#pragma once
#include "atomic.h"
#include "cap.h"

typedef enum cap_type {
        CAP_INVALID = 0,
        CAP_MEMORY_SLICE,
        CAP_PMP_ENTRY,
        CAP_TIME_SLICE,
        CAP_CHANNELS,
        CAP_ENDPOINT
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

typedef struct cap_time_slice {
        bool valid;
        uint8_t begin;
        uint8_t end;
        uint8_t tsid;
        uint8_t fuel;
} CapTimeSlice;

typedef struct cap_endpoint {
        bool valid;
        uint8_t epid;
        bool is_receiver;
} CapEndpoint;

typedef struct cap_channels {
        bool valid;
        uint8_t begin;
        uint8_t end;
} CapChannels;

/* Basic capability utilities */
static inline bool cap_is_deleted(const Cap *cap);
static inline bool cap_get_data(const Cap *cap, uint64_t data[2]);
static inline bool cap_set_data(Cap *cap, const uint64_t data[2]);
static inline CapType cap_get_type(const Cap *cap);

/* Memory slice */
static inline CapMemorySlice cap_mk_memory_slice(uint64_t begin, uint64_t end,
                                                 uint8_t rwx);
static inline CapMemorySlice cap_get_memory_slice(const Cap *cap);
static inline bool cap_set_memory_slice(Cap *cap, const CapMemorySlice ms);

/* PMP entry */
static inline CapPmpEntry cap_mk_pmp_entry(uint64_t addr, uint8_t rwx);
static inline CapPmpEntry cap_get_pmp_entry(const Cap *cap);
static inline bool cap_set_pmp_entry(Cap *cap, const CapPmpEntry pe);
static inline void cap_get_pmp_entry_bounds(const CapPmpEntry pe,
                                            uint64_t *begin, uint64_t *end);

/* Time slice */
static inline CapTimeSlice cap_mk_time_slice(uint8_t begin, uint8_t end,
                                             uint8_t tsid, uint8_t fuel);
static inline CapTimeSlice cap_get_time_slice(const Cap *cap);
static inline bool cap_set_time_slice(Cap *cap, const CapTimeSlice ms);

/* Endpoint */
static inline CapEndpoint cap_mk_endpoint(uint8_t epid, bool is_receiver);
static inline CapEndpoint cap_get_endpoint(const Cap *cap);
static inline bool cap_set_endpoint(Cap *cap, const CapEndpoint ep);

/* Channels */
static inline CapChannels cap_mk_channels(uint8_t begin, uint8_t end);
static inline CapChannels cap_get_channels(const Cap *cap);
static inline bool cap_set_channels(Cap *cap, const CapChannels ep);

/* Is child predicates */
static inline bool cap_is_child_ms_ms(const CapMemorySlice parent,
                                      const CapMemorySlice child);
static inline bool cap_is_child_ms_pe(const CapMemorySlice parent,
                                      const CapPmpEntry child);
static inline bool cap_is_child_ts_ts(const CapTimeSlice parent,
                                      const CapTimeSlice child);

static inline bool cap_is_child_ep_ep(const CapEndpoint parent,
                                      const CapEndpoint child);
static inline bool cap_is_child_ch_ep(const CapChannels parent,
                                      const CapEndpoint child);
static inline bool cap_is_child_ch_ch(const CapChannels parent,
                                      const CapChannels child);

bool cap_is_deleted(const Cap *cap) {
        return cap->next == NULL;
}

bool cap_get_data(const Cap *cap, uint64_t data[2]) {
        data[0] = cap->data[0];
        data[1] = cap->data[1];
        __sync_synchronize();
        return !cap_is_deleted(cap);
}

bool cap_set_data(Cap *cap, const uint64_t data[2]) {
        if (!cap_is_deleted(cap))
                return false;
        cap->data[0] = data[0];
        cap->data[1] = data[1];
        return true;
}

CapType cap_get_type(const Cap *cap) {
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

CapMemorySlice cap_get_memory_slice(const Cap *cap) {
        uint64_t data[2];
        if (!cap_get_data(cap, data) || (data[0] & 0xFF) != CAP_MEMORY_SLICE) {
                return (CapMemorySlice){.valid = false};
        }
        return cap_mk_memory_slice(data[0] >> 8, data[1] >> 8, data[1] & 0xFF);
}

bool cap_set_memory_slice(Cap *cap, CapMemorySlice ms) {
        uint64_t data[2];
        data[0] = CAP_MEMORY_SLICE;
        data[0] |= ms.begin << 8;
        data[1] = ms.rwx;
        data[1] |= ms.end << 8;
        return cap_set_data(cap, data);
}

CapPmpEntry cap_mk_pmp_entry(uint64_t addr, uint8_t rwx) {
        bool valid = ((addr >> 56) == 0) || rwx != 0;
        return (CapPmpEntry){.valid = valid, .addr = addr, .rwx = rwx};
}

CapPmpEntry cap_get_pmp_entry(const Cap *cap) {
        uint64_t data[2];
        if (!cap_get_data(cap, data) || (data[0] & 0xFF) != CAP_PMP_ENTRY) {
                return (CapPmpEntry){.valid = false};
        }
        return cap_mk_pmp_entry(data[0] >> 8, data[1] & 0xFF);
}

bool cap_set_pmp_entry(Cap *cap, const CapPmpEntry pe) {
        uint64_t data[2];
        data[0] = CAP_PMP_ENTRY;
        data[0] |= pe.addr << 8;
        data[1] = pe.rwx;
        return cap_set_data(cap, data);
}

void cap_get_pmp_entry_bounds(const CapPmpEntry pe, uint64_t *begin,
                              uint64_t *end) {
        *begin = pe.addr & (pe.addr + 1);
        uint64_t length = (pe.addr ^ (pe.addr + 1)) + 1;
        *end = *begin + length;
}

CapTimeSlice cap_mk_time_slice(uint8_t begin, uint8_t end, uint8_t tsid,
                               uint8_t fuel) {
        CapTimeSlice ts;
        ts.begin = begin;
        ts.end = end;
        ts.tsid = tsid;
        ts.fuel = fuel;
        ts.valid = (begin < end) && ((uint16_t)tsid + fuel < 256);
        return ts;
}

CapTimeSlice cap_get_time_slice(const Cap *cap) {
        uint64_t data[2];
        if (!cap_get_data(cap, data) || (data[0] & 0xFF) != CAP_TIME_SLICE) {
                return (CapTimeSlice){.valid = false};
        }
        uint8_t begin = data[0] >> 8;
        uint8_t end = data[0] >> 16;
        uint8_t tsid = data[0] >> 24;
        uint8_t fuel = data[0] >> 32;
        return cap_mk_time_slice(begin, end, tsid, fuel);
}

bool cap_set_time_slice(Cap *cap, const CapTimeSlice ts) {
        uint64_t data[2];
        data[0] = CAP_TIME_SLICE;
        data[0] |= (uint64_t)ts.begin << 8;
        data[0] |= (uint64_t)ts.end << 16;
        data[0] |= (uint64_t)ts.tsid << 24;
        data[0] |= (uint64_t)ts.fuel << 32;
        data[1] = 0;
        return cap_set_data(cap, data);
}

CapEndpoint cap_mk_endpoint(uint8_t epid, bool is_receiver) {
        return (CapEndpoint){
            .valid = true, .epid = epid, .is_receiver = is_receiver};
}

CapEndpoint cap_get_endpoint(const Cap *cap) {
        uint64_t data[2];
        if (!cap_get_data(cap, data) || (data[0] & 0xFF) != CAP_ENDPOINT) {
                return (CapEndpoint){.valid = false};
        }
        return cap_mk_endpoint(data[0] >> 8, data[0] >> 16);
}

bool cap_set_endpoint(Cap *cap, const CapEndpoint ep) {
        uint64_t data[2];
        data[0] = CAP_ENDPOINT;
        data[0] |= (uint64_t)ep.epid << 8;
        data[0] |= (uint64_t)ep.is_receiver << 16;
        data[1] = 0;
        return cap_set_data(cap, data);
}

CapChannels cap_mk_channels(uint8_t begin, uint8_t end) {
        return (CapChannels){.valid = begin < end, .begin = begin, .end = end};
}

CapChannels cap_get_channels(const Cap *cap) {
        uint64_t data[2];
        if (!cap_get_data(cap, data) || (data[0] & 0xFF) != CAP_ENDPOINT) {
                return (CapChannels){.valid = false};
        }
        return cap_mk_channels(data[0] >> 8, data[0] >> 16);
}

bool cap_set_channels(Cap *cap, const CapChannels ch) {
        uint64_t data[2];
        data[0] = CAP_ENDPOINT;
        data[0] |= (uint64_t)ch.begin << 8;
        data[0] |= (uint64_t)ch.end << 16;
        data[1] = 0;
        return cap_set_data(cap, data);
}

bool cap_is_child_ms_ms(const CapMemorySlice parent,
                        const CapMemorySlice child) {
        return parent.begin <= child.begin && child.end <= parent.end &&
               (child.rwx & parent.rwx) == child.rwx;
}

bool cap_is_child_ms_pe(const CapMemorySlice parent, const CapPmpEntry child) {
        uint64_t begin, end;
        cap_get_pmp_entry_bounds(child, &begin, &end);
        return parent.begin <= begin && end <= parent.end &&
               (child.rwx & parent.rwx) == child.rwx;
}

bool cap_is_child_ts_ts(const CapTimeSlice parent, const CapTimeSlice child) {
        return parent.begin <= child.begin && child.end <= parent.end &&
               parent.tsid < child.tsid &&
               parent.tsid + parent.fuel <= child.tsid + child.fuel;
}

bool cap_is_child_ep_ep(const CapEndpoint parent, const CapEndpoint child) {
        return parent.epid == child.epid && parent.is_receiver &&
               !child.is_receiver;
}

bool cap_is_child_ch_ep(const CapChannels parent, const CapEndpoint child) {
        return parent.begin <= child.epid && child.epid < parent.end;
}

bool cap_is_child_ch_ch(const CapChannels parent, const CapChannels child) {
        return parent.begin <= child.begin && child.end <= parent.end;
}
