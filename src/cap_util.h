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

typedef enum cap_rwx {
        CAP_R = 1,
        CAP_W = 2,
        CAP_RW = 3,
        CAP_X = 4,
        CAP_RX = 5,
        CAP_WX = 6,
        CAP_RWX = 7,
} CapRWX;

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

typedef struct cap_union {
        CapType type;
        CapMemorySlice memory_slice;
        CapPmpEntry pmp_entry;
        CapLoadedPmp loaded_pmp;
        CapTimeSlice time_slice;
        CapChannels channels;
        CapReceiver receiver;
        CapSender sender;
        CapSupervisor supervisor;
} CapUnion;

/* Basic capability utilities */
static inline bool cap_is_deleted(const Cap *cap);
static inline bool cap_get_data(const Cap *cap, uint64_t data[2]);
static inline bool cap_set_data(Cap *cap, const uint64_t data[2]);
static inline CapType cap_get_type(const Cap *cap);

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

/* Capability setters */
static inline bool cap_set(Cap *cap, const CapUnion cunion);
static inline bool cap_set_memory_slice(Cap *cap, const CapMemorySlice ms);
static inline bool cap_set_pmp_entry(Cap *cap, const CapPmpEntry pe);
static inline bool cap_set_loaded_pmp(Cap *cap, const CapLoadedPmp pe);
static inline bool cap_set_time_slice(Cap *cap, const CapTimeSlice ts);
static inline bool cap_set_channels(Cap *cap, const CapChannels ch);
static inline bool cap_set_sender(Cap *cap, const CapSender sender);
static inline bool cap_set_receiver(Cap *cap, const CapReceiver receiver);
static inline bool cap_set_supervisor(Cap *cap, const CapSupervisor sup);

/* Capability getters */
static inline CapUnion cap_get(const Cap *cap);
static inline CapUnion cap_get_union(const uint64_t data[2]);
static inline CapMemorySlice cap_get_memory_slice(const uint64_t data[2]);
static inline CapPmpEntry cap_get_pmp_entry(const uint64_t data[2]);
static inline CapLoadedPmp cap_get_loaded_pmp(const uint64_t data[2]);
static inline CapTimeSlice cap_get_time_slice(const uint64_t data[2]);
static inline CapChannels cap_get_channels(const uint64_t data[2]);
static inline CapSender cap_get_sender(const uint64_t data[2]);
static inline CapReceiver cap_get_receiver(const uint64_t data[2]);
static inline CapSupervisor cap_get_supervisor(const uint64_t data[2]);

/* Is child */
static inline bool cap_is_child(const CapUnion parent, const CapUnion child);
static inline bool cap_is_child_ms(const CapMemorySlice parent,
                                   const CapUnion child);
static inline bool cap_is_child_ms_ms(const CapMemorySlice parent,
                                      const CapMemorySlice child);
static inline bool cap_is_child_ms_pe(const CapMemorySlice parent,
                                      const CapPmpEntry child);
static inline bool cap_is_child_pe(const CapPmpEntry parent,
                                      const CapUnion child);
static inline bool cap_is_child_ts(const CapTimeSlice parent,
                                   const CapUnion child);
static inline bool cap_is_child_ts_ts(const CapTimeSlice parent,
                                      const CapTimeSlice child);
static inline bool cap_is_child_ch(const CapChannels parent,
                                   const CapUnion child);
static inline bool cap_is_child_ch_ch(const CapChannels parent,
                                      const CapChannels child);
static inline bool cap_is_child_ch_recv(const CapChannels parent,
                                        const CapReceiver child);
static inline bool cap_is_child_ch_send(const CapChannels parent,
                                        const CapSender child);

bool cap_is_deleted(const Cap *cap) {
        return cap->next == NULL;
}

bool cap_get_data(const Cap *cap, uint64_t data[2]) {
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

bool cap_set_data(Cap *cap, const uint64_t data[2]) {
        if (cap_is_deleted(cap)) {
                cap->data[0] = data[0];
                cap->data[1] = data[1];
                return true;
        }
        return false;
}

static inline CapType cap_get_type(const Cap *cap) {
        uint64_t data[2];
        cap_get_data(cap, data);
        return data[0] & 0xFF;
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

bool cap_set(Cap *cap, const CapUnion cunion) {
        uint64_t data[2];
        data[0] = data[1] = 0;
        switch (cunion.type) {
                case CAP_EMPTY:
                        break;
                case CAP_MEMORY_SLICE:
                        CapMemorySlice ms = cunion.memory_slice;
                        data[0] = CAP_MEMORY_SLICE | ms.begin << 8;
                        data[1] = ms.end << 8 | ms.rwx;
                        break;
                case CAP_PMP_ENTRY:
                        CapPmpEntry pe = cunion.pmp_entry;
                        data[0] = CAP_PMP_ENTRY | pe.addr << 8;
                        data[1] = pe.rwx;
                        break;
                case CAP_LOADED_PMP:
                        CapLoadedPmp lp = cunion.loaded_pmp;
                        data[0] = CAP_LOADED_PMP;
                        data[1] = lp.addr << 8 | lp.cfg;
                        break;
                case CAP_TIME_SLICE:
                        CapTimeSlice ts = cunion.time_slice;
                        data[0] = CAP_TIME_SLICE | ts.hartid << 8 |
                                  ts.begin << 16 | (uint64_t)ts.end << 32 |
                                  (uint64_t)ts.tsid << 48 |
                                  (uint64_t)ts.tsid_end << 56;
                        data[1] = 0;
                        break;
                case CAP_CHANNELS:
                        CapChannels ch = cunion.channels;
                        data[0] = CAP_CHANNELS | ch.begin << 8 | ch.end << 16;
                        data[1] = 0;
                        break;
                case CAP_RECEIVER:
                        CapReceiver receiver = cunion.receiver;
                        data[0] = CAP_RECEIVER | receiver.channel << 8;
                        data[1] = 0;
                        break;
                case CAP_SENDER:
                        CapSender sender = cunion.sender;
                        data[0] = CAP_SENDER | sender.channel << 8;
                        data[1] = 0;
                        break;
                case CAP_SUPERVISOR:
                        CapSupervisor sup = cunion.supervisor;
                        data[0] = CAP_SUPERVISOR | sup.pid << 8;
                        data[1] = 0;
                        break;
        }
        return cap_set_data(cap, data);
}

bool cap_set_memory_slice(Cap *cap, const CapMemorySlice ms) {
        return cap_set(
            cap, (CapUnion){.memory_slice = ms, .type = CAP_MEMORY_SLICE});
}

bool cap_set_pmp_entry(Cap *cap, const CapPmpEntry pe) {
        return cap_set(cap, (CapUnion){.pmp_entry = pe, .type = CAP_PMP_ENTRY});
}

bool cap_set_loaded_pmp(Cap *cap, const CapLoadedPmp lp) {
        return cap_set(cap,
                       (CapUnion){.loaded_pmp = lp, .type = CAP_LOADED_PMP});
}

bool cap_set_time_slice(Cap *cap, const CapTimeSlice ts) {
        return cap_set(cap,
                       (CapUnion){.time_slice = ts, .type = CAP_TIME_SLICE});
}

bool cap_set_channels(Cap *cap, const CapChannels ch) {
        return cap_set(cap, (CapUnion){.channels = ch, .type = CAP_CHANNELS});
}

bool cap_set_receiver(Cap *cap, const CapReceiver receiver) {
        return cap_set(cap,
                       (CapUnion){.receiver = receiver, .type = CAP_RECEIVER});
}

bool cap_set_sender(Cap *cap, const CapSender sender) {
        return cap_set(cap, (CapUnion){.sender = sender, .type = CAP_SENDER});
}

bool cap_set_supervisor(Cap *cap, const CapSupervisor sup) {
        return cap_set(cap,
                       (CapUnion){.supervisor = sup, .type = CAP_SUPERVISOR});
}

CapUnion cap_get(const Cap *cap) {
        uint64_t data[2];
        cap_get_data(cap, data);
        return cap_get_union(data);
}

CapUnion cap_get_union(const uint64_t data[2]) {
        return (CapUnion){data[0] & 0xFF,           cap_get_memory_slice(data),
                          cap_get_pmp_entry(data),  cap_get_loaded_pmp(data),
                          cap_get_time_slice(data), cap_get_channels(data),
                          cap_get_receiver(data),   cap_get_sender(data),
                          cap_get_supervisor(data)};
}

CapMemorySlice cap_get_memory_slice(const uint64_t data[2]) {
        return cap_mk_memory_slice(data[0] >> 8, data[1] >> 8, data[1] & 0xFF);
}

CapPmpEntry cap_get_pmp_entry(const uint64_t data[2]) {
        return cap_mk_pmp_entry(data[0] >> 8, data[1] & 0x7);
}

CapLoadedPmp cap_get_loaded_pmp(const uint64_t data[2]) {
        return cap_mk_loaded_pmp(data[1] >> 8, data[1] & 0xFF);
}

CapTimeSlice cap_get_time_slice(const uint64_t data[2]) {
        return cap_mk_time_slice(data[0] >> 8, data[0] >> 16, data[0] >> 32,
                                 data[0] >> 48, data[0] >> 56);
}

CapChannels cap_get_channels(const uint64_t data[2]) {
        return cap_mk_channels(data[0] >> 8, data[0] >> 16);
}

CapReceiver cap_get_receiver(const uint64_t data[2]) {
        return cap_mk_receiver(data[0] >> 8);
}

CapSender cap_get_sender(const uint64_t data[2]) {
        return cap_mk_sender(data[0] >> 8);
}

CapSupervisor cap_get_supervisor(const uint64_t data[2]) {
        return cap_mk_supervisor(data[0] >> 8);
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

bool cap_is_child_ms(const CapMemorySlice parent, const CapUnion child) {
        switch (child.type) {
                case CAP_MEMORY_SLICE:
                        return cap_is_child_ms_ms(parent, child.memory_slice);
                case CAP_PMP_ENTRY:
                        return cap_is_child_ms_pe(parent, child.pmp_entry);
                case CAP_LOADED_PMP:
                        return true;
                default:
                        return false;
        }
}

bool cap_is_child_pe(const CapPmpEntry parent, const CapUnion child) {
        return child.type == CAP_LOADED_PMP;
}

bool cap_is_child_ts_ts(const CapTimeSlice parent, const CapTimeSlice child) {
        return parent.begin <= child.begin && child.end <= parent.end &&
               parent.tsid < child.tsid && child.tsid_end <= parent.tsid_end;
}

bool cap_is_child_ts(const CapTimeSlice parent, const CapUnion child) {
        switch (child.type) {
                case CAP_TIME_SLICE:
                        return cap_is_child_ts_ts(parent, child.time_slice);
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

bool cap_is_child_ch(const CapChannels parent, const CapUnion child) {
        switch (child.type) {
                case CAP_CHANNELS:
                        return cap_is_child_ch_ch(parent, child.channels);
                case CAP_RECEIVER:
                        return cap_is_child_ch_recv(parent, child.receiver);
                case CAP_SENDER:
                        return cap_is_child_ch_send(parent, child.sender);
                default:
                        return false;
        }
}

bool cap_is_child(const CapUnion parent, const CapUnion child) {
        switch (parent.type) {
                case CAP_MEMORY_SLICE:
                        return cap_is_child_ms(parent.memory_slice, child);
                case CAP_TIME_SLICE:
                        return cap_is_child_ts(parent.time_slice, child);
                case CAP_CHANNELS:
                        return cap_is_child_ch(parent.channels, child);
                case CAP_PMP_ENTRY:
                        return cap_is_child_pe(parent.pmp_entry, child);
                default:
                        return false;
        }
        return false;
}
