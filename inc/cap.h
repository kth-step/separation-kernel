// See LICENSE file for copyright and license details.
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "assert.h"
#include "config.h"
#include "pmp.h"

typedef struct cap Cap;
typedef enum cap_type CapType;
typedef struct cap_node CapNode;

struct cap {
        uint64_t word0, word1;
};

struct cap_node {
        CapNode *prev, *next;
        uint64_t word0, word1;
};

enum cap_type {
        CAP_TYPE_INVALID = 0,
        CAP_TYPE_MEMORY_SLICE = 1,
        CAP_TYPE_PMP_ENTRY = 2,
        CAP_TYPE_TIME_SLICE = 3,
        CAP_TYPE_CHANNELS = 4,
        CAP_TYPE_ENDPOINT = 5,
        CAP_TYPE_SUPERVISOR = 6,
        CAP_TYPE_PMP_ENTRY_HIDDEN = 16,
};

extern CapNode cap_tables[N_PROC][N_CAPS];
extern volatile int ep_receiver[N_CHANNELS];

/* Delete all children of node */
bool CapRevoke(CapNode *node);
/* Delete node */
bool CapDelete(CapNode *node);
/* Move node in src to dest */
bool CapMove(CapNode *src, CapNode *dest);
/* Insert node after parent, if insertion is successful, set the data to cap */
bool CapInsert(const Cap cap, CapNode *node, CapNode *parent);
/* Make a sentinel node. */
CapNode *CapInitSentinel(void);

/* Check if a node has been deleted */
static inline bool cap_is_deleted(const CapNode *cn) {
        // Check if prev is NULL ?
        return cn->next == NULL;
}

static inline Cap cn_get(const CapNode *cn) {
        Cap cap;
        cap.word0 = cn->word0;
        cap.word1 = cn->word1;
        __sync_synchronize();
        if (cap_is_deleted(cn)) {
                cap.word0 = 0;
                cap.word1 = 0;
        }
        return cap;
}

static inline bool cn_set(CapNode *cn, const Cap cap) {
        if (cap_is_deleted(cn))
                return false;
        cn->word0 = cap.word0;
        cn->word1 = cap.word1;
        return true;
}

static inline CapType cap_get_type(const Cap cap) {
        return cap.word0 & 0xFF;
}

static inline Cap cap_memory_slice(uint64_t begin, uint64_t end, uint64_t rwx) {
        ASSERT((begin & 0xFFFFFFFFFFFFFFull) == begin);
        ASSERT((end & 0xFFFFFFFFFFFFFFull) == end);
        ASSERT(begin < end);
        ASSERT(rwx > 0 && rwx < 8);
        return (Cap){CAP_TYPE_MEMORY_SLICE | begin << 8, rwx | end << 8};
}

static inline uint64_t cap_memory_slice_begin(const Cap cap) {
        ASSERT((cap.word0 & 0xFF) == CAP_TYPE_MEMORY_SLICE);
        return cap.word0 >> 8;
}

static inline uint64_t cap_memory_slice_end(const Cap cap) {
        ASSERT((cap.word0 & 0xFF) == CAP_TYPE_MEMORY_SLICE);
        return cap.word1 >> 8;
}

static inline uint64_t cap_memory_slice_rwx(const Cap cap) {
        ASSERT((cap.word0 & 0xFF) == CAP_TYPE_MEMORY_SLICE);
        return cap.word1 & 0xFF;
}

static inline Cap cap_pmp_entry(uint64_t addr, uint64_t rwx) {
        ASSERT((addr & 0xFFFFFFFFFFFFFFull) == addr);
        ASSERT(rwx > 0 && rwx < 8);
        return (Cap){CAP_TYPE_PMP_ENTRY, addr << 8 | rwx};
}

static inline uint64_t cap_pmp_entry_addr(const Cap cap) {
        return cap.word1 >> 8;
}

static inline uint64_t cap_pmp_entry_rwx(const Cap cap) {
        return cap.word1 & 0xFF;
}

static inline Cap cap_pmp_entry_hidden(uint64_t addr, uint64_t rwx) {
        ASSERT((addr & 0xFFFFFFFFFFFFFFull) == addr);
        ASSERT(rwx > 0 && rwx < 8);
        return (Cap){CAP_TYPE_PMP_ENTRY_HIDDEN, addr << 8 | 0x18 | rwx};
}

static inline uint64_t cap_pmp_entry_hidden_addr(const Cap cap) {
        return cap.word1 >> 8;
}

static inline uint64_t cap_pmp_entry_hidden_cfg(const Cap cap) {
        return cap.word1 & 0xFF;
}

static inline Cap cap_time_slice(uint64_t core, uint64_t begin, uint64_t end,
                                 uint64_t id, uint64_t fuel) {
        ASSERT((core & 0xFF) == core);
        ASSERT((begin & 0xFFFF) == begin);
        ASSERT((end & 0xFFFF) == end);
        ASSERT((id & 0xFF) == id);
        ASSERT((fuel & 0xFF) == fuel);
        ASSERT(begin < end);
        ASSERT(id <= fuel);
        return (Cap){CAP_TYPE_TIME_SLICE | core << 8 | begin << 16 | end << 32 |
                         id << 48 | fuel << 56,
                     0};
}

static inline uint64_t cap_time_slice_core(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_TIME_SLICE);
        return (cap.word0 >> 8) & 0xFF;
}

static inline uint64_t cap_time_slice_begin(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_TIME_SLICE);
        return (cap.word0 >> 16) & 0xFFFF;
}

static inline uint64_t cap_time_slice_end(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_TIME_SLICE);
        return (cap.word0 >> 32) & 0xFFFF;
}

static inline uint64_t cap_time_slice_id(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_TIME_SLICE);
        return (cap.word0 >> 48) & 0xFF;
}

static inline uint64_t cap_time_slice_fuel(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_TIME_SLICE);
        return (cap.word0 >> 56) & 0xFF;
}

static inline Cap cap_channels(uint64_t begin, uint64_t end) {
        ASSERT((begin & 0xFFFF) == begin);
        ASSERT((end & 0xFFFF) == end);
        ASSERT(begin < end);
        return (Cap){CAP_TYPE_CHANNELS | begin << 16 | end << 32, 0};
}

static inline uint64_t cap_channels_begin(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_CHANNELS);
        return (cap.word0 >> 16) & 0xFFFF;
}

static inline uint64_t cap_channels_end(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_CHANNELS);
        return (cap.word0 >> 32) & 0xFFFF;
}

static inline Cap cap_endpoint(uint64_t channel, uint64_t receive,
                               uint64_t send, uint64_t grant) {
        ASSERT((channel & 0xFFFF) == channel);
        ASSERT((receive & 0xFF) == receive);
        ASSERT((send & 0xFF) == send);
        ASSERT((grant & 0xFF) == grant);
        ASSERT(!grant || send);
        return (Cap){CAP_TYPE_ENDPOINT | channel << 16 | receive << 32 |
                         send << 40 | grant << 48,
                     0};
}

static inline uint64_t cap_endpoint_channel(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_ENDPOINT);
        return (cap.word0 >> 16) & 0xFFFF;
}

static inline uint64_t cap_endpoint_receive(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_ENDPOINT);
        return (cap.word0 >> 32) & 0xFF;
}

static inline uint64_t cap_endpoint_send(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_ENDPOINT);
        return (cap.word0 >> 40) & 0xFF;
}

static inline uint64_t cap_endpoint_grant(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_ENDPOINT);
        return (cap.word0 >> 48) & 0xFF;
}

static inline Cap cap_supervisor(uint64_t pid) {
        ASSERT((pid & 0x7F) == pid);
        return (Cap){CAP_TYPE_SUPERVISOR | pid << 8, 0};
}

static inline uint64_t cap_supervisor_pid(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_TYPE_SUPERVISOR);
        return (cap.word0 >> 8) & 0x7F;
}

static inline bool cap_is_child_ms_ms(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_TYPE_MEMORY_SLICE);
        ASSERT(cap_get_type(child) == CAP_TYPE_MEMORY_SLICE);
        return cap_memory_slice_begin(parent) <=
                   cap_memory_slice_begin(child) &&
               cap_memory_slice_end(child) <= cap_memory_slice_end(parent);
}

static inline bool cap_is_child_ms_pe(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_TYPE_MEMORY_SLICE);
        ASSERT(cap_get_type(child) == CAP_TYPE_PMP_ENTRY);
        uint64_t child_begin, child_end;
        uint64_t addr = cap_pmp_entry_addr(child);
        pmp_napot_bounds(addr, &child_begin, &child_end);
        uint64_t parent_rwx = cap_memory_slice_rwx(parent);
        uint64_t child_rwx = cap_pmp_entry_rwx(child);
        return cap_memory_slice_begin(parent) <= child_begin &&
               child_end <= cap_memory_slice_end(parent) &&
               (child_rwx & parent_rwx) == child_rwx;
}

static inline bool cap_is_child_ms(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_TYPE_MEMORY_SLICE);
        switch (cap_get_type(child)) {
                case CAP_TYPE_MEMORY_SLICE:
                        return cap_is_child_ms_ms(parent, child);
                case CAP_TYPE_PMP_ENTRY:
                        return cap_is_child_ms_pe(parent, child);
                case CAP_TYPE_PMP_ENTRY_HIDDEN:
                        return true;
                default:
                        return false;
        }
}

static inline bool cap_is_child_ts_ts(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_TYPE_TIME_SLICE);
        ASSERT(cap_get_type(child) == CAP_TYPE_TIME_SLICE);
        return cap_time_slice_core(parent) == cap_time_slice_core(child) &&
               cap_time_slice_begin(parent) <= cap_time_slice_begin(child) &&
               cap_time_slice_end(child) <= cap_time_slice_end(child) &&
               cap_time_slice_id(parent) < cap_time_slice_id(child) &&
               cap_time_slice_fuel(child) <= cap_time_slice_fuel(parent);
}

static inline bool cap_is_child_ts(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_TYPE_TIME_SLICE);
        switch (cap_get_type(child)) {
                case CAP_TYPE_TIME_SLICE:
                        return cap_is_child_ts_ts(parent, child);
                default:
                        return false;
        }
}

static inline bool cap_is_child_pe(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_TYPE_PMP_ENTRY);
        return cap_get_type(child) == CAP_TYPE_PMP_ENTRY_HIDDEN;
}

static inline bool cap_is_child_ch_ch(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_TYPE_CHANNELS);
        ASSERT(cap_get_type(child) == CAP_TYPE_CHANNELS);
        return cap_channels_begin(parent) <= cap_channels_begin(child) &&
               cap_channels_end(child) <= cap_channels_end(parent);
}

static inline bool cap_is_child_ch_ep(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_TYPE_CHANNELS);
        ASSERT(cap_get_type(child) == CAP_TYPE_ENDPOINT);
        return cap_channels_begin(parent) <= cap_endpoint_channel(child) &&
               cap_endpoint_channel(child) < cap_channels_end(parent);
}

static inline bool cap_is_child_ch(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_TYPE_CHANNELS);
        switch (cap_get_type(child)) {
                case CAP_TYPE_CHANNELS:
                        return cap_is_child_ch_ch(parent, child);
                case CAP_TYPE_ENDPOINT:
                        return cap_is_child_ch_ep(parent, child);
                default:
                        return false;
        }
}

static inline bool cap_is_child(const Cap parent, const Cap child) {
        switch (cap_get_type(parent)) {
                case CAP_TYPE_MEMORY_SLICE:
                        return cap_is_child_ms(parent, child);
                case CAP_TYPE_TIME_SLICE:
                        return cap_is_child_ts(parent, child);
                case CAP_TYPE_PMP_ENTRY:
                        return cap_is_child_pe(parent, child);
                case CAP_TYPE_CHANNELS:
                        return cap_is_child_ch(parent, child);
                default:
                        return false;
        }
}
