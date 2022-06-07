// See LICENSE file for copyright and license details.
#pragma once
#include "cap.h"
#include "pmp.h"

typedef enum cap_type {
        CAP_EMPTY = 0,
        CAP_MEMORY_SLICE,
        CAP_PMP_ENTRY,
        CAP_TIME_SLICE,
        CAP_CHANNELS,
        CAP_RECEIVER,
        CAP_SENDER,
        CAP_SUPERVISOR,
        /* Hidden capabilities, not in ordinary capability table */
        CAP_LOADED_PMP, /* Stored in PMP capability table */
} CapType;

typedef struct cap_memory_slice {
        uint64_t begin;
        uint64_t end;
        uint8_t rwx;
} CapMemorySlice;

typedef struct cap_pmp_entry {
        uint64_t addr;
        uint8_t rwx;
} CapPmpEntry;

typedef struct cap_loaded_pmp {
        uint64_t addr;
        uint8_t cfg;
} CapLoadedPmp;

typedef struct cap_time_slice {
        uint8_t hartid;
        uint16_t begin;
        uint16_t end;
        uint8_t tsid;
        uint8_t tsid_end;
} CapTimeSlice;

typedef struct cap_receiver {
        uint16_t channel;
} CapReceiver;

typedef struct cap_sender {
        uint16_t channel;
} CapSender;

typedef struct cap_channels {
        uint8_t begin;
        uint8_t end;
} CapChannels;

typedef struct cap_supervisor {
        int8_t pid;
} CapSupervisor;

typedef struct cap_data {
        uint64_t data0, data1;
} CapData;

/* Basic capability utilities */
static inline bool cap_is_deleted(const Cap *cap);
static inline CapData cap_get(const Cap *cap);
static inline bool cap_set(Cap *cap, const CapData);
static inline bool cap_get_arr(const Cap *cap, uint64_t data[2]);
static inline bool cap_set_arr(Cap *cap, const uint64_t data[2]);
static inline CapType cap_get_type(const CapData cd);

/* Capability makers */
static inline CapMemorySlice cap_mk_memory_slice(uint64_t begin, uint64_t end,
                                                 uint8_t rwx);
static inline CapPmpEntry cap_mk_pmp_entry(uint64_t addr, uint8_t rwx);
static inline CapLoadedPmp cap_mk_loaded_pmp(uint64_t addr, uint8_t rwx);
static inline CapTimeSlice cap_mk_time_slice(uint8_t hartid, uint16_t begin,
                                             uint16_t end, uint8_t tsid,
                                             uint8_t tsid_end);
static inline CapChannels cap_mk_channels(uint8_t begin, uint8_t end);
static inline CapSender cap_mk_sender(uint8_t channel);
static inline CapReceiver cap_mk_receiver(uint8_t channel);
static inline CapSupervisor cap_mk_supervisor(uint8_t pid);

/* Capability serialization */
static inline CapData cap_serialize_memory_slice(const CapMemorySlice ms);
static inline CapData cap_serialize_pmp_entry(const CapPmpEntry pe);
static inline CapData cap_serialize_loaded_pmp(const CapLoadedPmp pe);
static inline CapData cap_serialize_time_slice(const CapTimeSlice ts);
static inline CapData cap_serialize_channels(const CapChannels ch);
static inline CapData cap_serialize_sender(const CapSender sender);
static inline CapData cap_serialize_receiver(const CapReceiver receiver);
static inline CapData cap_serialize_supervisor(const CapSupervisor sup);

/* Capability deserialization */
static inline CapMemorySlice cap_deserialize_memory_slice(const CapData);
static inline CapPmpEntry cap_deserialize_pmp_entry(const CapData);
static inline CapLoadedPmp cap_deserialize_loaded_pmp(const CapData);
static inline CapTimeSlice cap_deserialize_time_slice(const CapData);
static inline CapChannels cap_deserialize_channels(const CapData);
static inline CapSender cap_deserialize_sender(const CapData);
static inline CapReceiver cap_deserialize_receiver(const CapData);
static inline CapSupervisor cap_deserialize_supervisor(const CapData);

/* Is child */
static inline bool cap_is_child(const CapData parent, const CapData child);
static inline bool cap_is_child_ms(const CapMemorySlice parent,
                                   const CapData child);
static inline bool cap_is_child_ms_ms(const CapMemorySlice parent,
                                      const CapMemorySlice child);
static inline bool cap_is_child_ms_pe(const CapMemorySlice parent,
                                      const CapPmpEntry child);
static inline bool cap_is_child_pe(const CapPmpEntry parent,
                                   const CapData child);
static inline bool cap_is_child_ts(const CapTimeSlice parent,
                                   const CapData child);
static inline bool cap_is_child_ts_ts(const CapTimeSlice parent,
                                      const CapTimeSlice child);
static inline bool cap_is_child_ch(const CapChannels parent,
                                   const CapData child);
static inline bool cap_is_child_ch_ch(const CapChannels parent,
                                      const CapChannels child);
static inline bool cap_is_child_ch_recv(const CapChannels parent,
                                        const CapReceiver child);
static inline bool cap_is_child_ch_send(const CapChannels parent,
                                        const CapSender child);

bool cap_is_deleted(const Cap *cap) {
        return cap->next == NULL;
}

bool cap_get_arr(const Cap *cap, uint64_t data[2]) {
        data[0] = cap->data[0];
        data[1] = cap->data[1];
        __sync_synchronize();
        if (cap_is_deleted(cap)) {
                data[0] = 0;
                data[1] = 0;
                return false;
        }
        return true;
}

bool cap_set_arr(Cap *cap, const uint64_t data[2]) {
        if (cap_is_deleted(cap)) {
                cap->data[0] = data[0];
                cap->data[1] = data[1];
                return true;
        }
        return false;
}

CapData cap_get(const Cap *cap) {
        uint64_t data[2];
        cap_get_arr(cap, data);
        return (CapData){.data0 = data[0], .data1 = data[1]};
}

bool cap_set(Cap *cap, const CapData cd) {
        uint64_t data[2];
        data[0] = cd.data0;
        data[1] = cd.data1;
        return cap_set_arr(cap, data);
}

static inline CapType cap_get_type(const CapData cd) {
        return cd.data0 & 0xFF;
}

CapMemorySlice cap_mk_memory_slice(uint64_t begin, uint64_t end, uint8_t rwx) {
        return (CapMemorySlice){begin, end, rwx};
}

CapPmpEntry cap_mk_pmp_entry(uint64_t addr, uint8_t rwx) {
        return (CapPmpEntry){addr, rwx};
}

CapLoadedPmp cap_mk_loaded_pmp(uint64_t addr, uint8_t rwx) {
        return (CapLoadedPmp){addr, 0x18 | rwx};
}

CapTimeSlice cap_mk_time_slice(uint8_t hartid, uint16_t begin, uint16_t end,
                               uint8_t tsid, uint8_t tsid_end) {
        return (CapTimeSlice){hartid, begin, end, tsid, tsid_end};
}

CapReceiver cap_mk_receiver(uint8_t channel) {
        return (CapReceiver){channel};
}

CapSender cap_mk_sender(uint8_t channel) {
        return (CapSender){channel};
}
CapChannels cap_mk_channels(uint8_t begin, uint8_t end) {
        return (CapChannels){begin, end};
}

CapSupervisor cap_mk_supervisor(uint8_t pid) {
        return (CapSupervisor){pid};
}

CapData cap_serialize_memory_slice(const CapMemorySlice ms) {
        return (CapData){.data0 = CAP_MEMORY_SLICE | ms.begin << 8,
                         .data1 = ms.end << 8 | ms.rwx};
}

CapData cap_serialize_pmp_entry(const CapPmpEntry pe) {
        return (CapData){.data0 = CAP_PMP_ENTRY | pe.addr << 8,
                         .data1 = pe.rwx};
}

CapData cap_serialize_loaded_pmp(const CapLoadedPmp lp) {
        return (CapData){.data0 = CAP_LOADED_PMP,
                         .data1 = lp.addr << 8 | lp.cfg};
}

CapData cap_serialize_time_slice(const CapTimeSlice ts) {
        return (CapData){.data0 = CAP_TIME_SLICE | ts.hartid << 8 |
                                  ts.begin << 16 | (uint64_t)ts.end << 32 |
                                  (uint64_t)ts.tsid << 48 |
                                  (uint64_t)ts.tsid_end << 56,
                         .data1 = 0};
}

CapData cap_serialize_channels(const CapChannels ch) {
        return (CapData){.data0 = CAP_CHANNELS | ch.begin << 8 | ch.end << 16,
                         .data1 = 0};
}

CapData cap_serialize_receiver(const CapReceiver receiver) {
        return (CapData){.data0 = CAP_RECEIVER | receiver.channel << 8,
                         .data1 = 0};
}

CapData cap_serialize_sender(const CapSender sender) {
        return (CapData){.data0 = CAP_SENDER | sender.channel << 8, .data1 = 0};
}

CapData cap_serialize_supervisor(const CapSupervisor sup) {
        return (CapData){.data0 = CAP_SUPERVISOR | sup.pid << 8, .data1 = 0};
}

CapMemorySlice cap_deserialize_memory_slice(const CapData cd) {
        return cap_mk_memory_slice(cd.data0 >> 8, cd.data1 >> 8,
                                   cd.data1 & 0xFF);
}

CapPmpEntry cap_deserialize_pmp_entry(const CapData cd) {
        return cap_mk_pmp_entry(cd.data0 >> 8, cd.data1 & 0x7);
}

CapLoadedPmp cap_deserialize_loaded_pmp(const CapData cd) {
        return cap_mk_loaded_pmp(cd.data1 >> 8, cd.data1 & 0xFF);
}

CapTimeSlice cap_deserialize_time_slice(const CapData cd) {
        return cap_mk_time_slice(cd.data0 >> 8, cd.data0 >> 16, cd.data0 >> 32,
                                 cd.data0 >> 48, cd.data0 >> 56);
}

CapChannels cap_deserialize_channels(const CapData cd) {
        return cap_mk_channels(cd.data0 >> 8, cd.data0 >> 16);
}

CapReceiver cap_deserialize_receiver(const CapData cd) {
        return cap_mk_receiver(cd.data0 >> 8);
}

CapSender cap_deserialize_sender(const CapData cd) {
        return cap_mk_sender(cd.data0 >> 8);
}

CapSupervisor cap_deserialize_supervisor(const CapData cd) {
        return cap_mk_supervisor(cd.data0 >> 8);
}

bool cap_is_child_ms_ms(const CapMemorySlice parent,
                        const CapMemorySlice child) {
        return parent.begin <= child.begin && child.end <= parent.end &&
               (child.rwx & parent.rwx) == child.rwx;
}

bool cap_is_child_ms_pe(const CapMemorySlice parent, const CapPmpEntry child) {
        uint64_t begin, end;
        pmp_napot_bounds(child.addr, &begin, &end);
        return parent.begin <= begin && end <= parent.end &&
               (child.rwx & parent.rwx) == child.rwx;
}

bool cap_is_child_ms(const CapMemorySlice parent, const CapData child) {
        switch (cap_get_type(child)) {
                case CAP_MEMORY_SLICE:
                        return cap_is_child_ms_ms(
                            parent, cap_deserialize_memory_slice(child));
                case CAP_PMP_ENTRY:
                        return cap_is_child_ms_pe(
                            parent, cap_deserialize_pmp_entry(child));
                case CAP_LOADED_PMP:
                        return true;
                default:
                        return false;
        }
}

bool cap_is_child_pe(const CapPmpEntry parent, const CapData child) {
        return cap_get_type(child) == CAP_LOADED_PMP;
}

bool cap_is_child_ts_ts(const CapTimeSlice parent, const CapTimeSlice child) {
        return parent.begin <= child.begin && child.end <= parent.end &&
               parent.tsid < child.tsid && child.tsid_end <= parent.tsid_end;
}

bool cap_is_child_ts(const CapTimeSlice parent, const CapData child) {
        switch (cap_get_type(child)) {
                case CAP_TIME_SLICE:
                        return cap_is_child_ts_ts(
                            parent, cap_deserialize_time_slice(child));
                default:
                        return false;
        }
}

bool cap_is_child_ch_ch(const CapChannels parent, const CapChannels child) {
        return parent.begin <= child.begin && child.end <= parent.end;
}

bool cap_is_child_ch_recv(const CapChannels parent, const CapReceiver child) {
        return parent.begin <= child.channel && child.channel <= parent.end;
}

bool cap_is_child_ch_send(const CapChannels parent, const CapSender child) {
        return parent.begin <= child.channel && child.channel <= parent.end;
}

bool cap_is_child_ch(const CapChannels parent, const CapData child) {
        switch (cap_get_type(child)) {
                case CAP_CHANNELS:
                        return cap_is_child_ch_ch(
                            parent, cap_deserialize_channels(child));
                case CAP_RECEIVER:
                        return cap_is_child_ch_recv(
                            parent, cap_deserialize_receiver(child));
                case CAP_SENDER:
                        return cap_is_child_ch_send(
                            parent, cap_deserialize_sender(child));
                default:
                        return false;
        }
}

bool cap_is_child(const CapData parent, const CapData child) {
        switch (cap_get_type(parent)) {
                case CAP_MEMORY_SLICE:
                        return cap_is_child_ms(
                            cap_deserialize_memory_slice(parent), child);
                case CAP_TIME_SLICE:
                        return cap_is_child_ts(
                            cap_deserialize_time_slice(parent), child);
                case CAP_CHANNELS:
                        return cap_is_child_ch(cap_deserialize_channels(parent),
                                               child);
                case CAP_PMP_ENTRY:
                        return cap_is_child_pe(
                            cap_deserialize_pmp_entry(parent), child);
                default:
                        return false;
        }
        return false;
}
