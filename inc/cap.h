// See LICENSE file for copyright and license details.
#pragma once

#include "assert.h"
#include "config.h"
#include "pmp.h"
#include "types.h"

#define NULL_CAP ((Cap){0, 0})

typedef struct cap Cap;
typedef enum cap_type CapType;
typedef struct cap_node CapNode;

struct cap {
        uint64_t word0, word1;
};

struct cap_node {
        CapNode *prev, *next;
        Cap cap;
};

enum cap_type {
        CAP_INVALID = 0,
        CAP_MEMORY = 1,
        CAP_PMP = 2,
        CAP_TIME = 3,
        CAP_CHANNELS = 4,
        CAP_ENDPOINT = 5,
        CAP_SUPERVISOR = 6,
        CAP_PMP_HIDDEN = 16,
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
/* Updates the cap in the node */
bool CapUpdate(const Cap cap, CapNode *node);
/* Make a sentinel node. */
CapNode *CapInitSentinel(void);

static inline Cap __mk_cap(CapType type, uint64_t word0, uint64_t word1) {
        ASSERT((word0 & 0xFFFFFFFFFFFFFFull) == word0);
        return (Cap){((uint64_t)type) << 56 | word0, word1};
}

/* Check if a node has been deleted */
static inline bool cap_is_deleted(const CapNode *cn) {
        // Check if prev is NULL ?
        return cn->next == NULL;
}

static inline Cap cn_get(const CapNode *cn) {
        Cap cap = cn->cap;
        __sync_synchronize();
        if (cap_is_deleted(cn)) {
                return NULL_CAP;
        }
        return cap;
}

static inline CapType cap_get_type(const Cap cap) {
        return (cap.word0 >> 56) & 0xFF;
}

static inline Cap cap_mk_memory(uint64_t begin, uint64_t end, uint64_t free,
                                uint64_t rwx, uint64_t pmp) {
        ASSERT((begin & 0xFFFFFFFFull) == begin);
        ASSERT((end & 0xFFFFFFFFull) == end);
        ASSERT((free & 0xFFFFFFFFull) == free);
        ASSERT(begin <= end);
        ASSERT(begin <= free);
        ASSERT(free <= end);
        ASSERT(rwx > 0 && rwx < 8);
        ASSERT((pmp & 1) == pmp);
        return __mk_cap(CAP_MEMORY, pmp << 40 | free | rwx << 32,
                        begin << 32 | end);
}

static inline uint64_t cap_memory_begin(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_MEMORY);
        return cap.word1 & 0xFFFFFFFFull;
}

static inline uint64_t cap_memory_end(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_MEMORY);
        return cap.word1 >> 32;
}

static inline uint64_t cap_memory_free(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_MEMORY);
        return cap.word0 & 0xFFFFFFFFull;
}

static inline uint64_t cap_memory_rwx(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_MEMORY);
        return (cap.word0 >> 8) & 0xFF;
}

static inline uint64_t cap_memory_pmp(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_MEMORY);
        return (cap.word0 >> 8) & 0xFF;
}

static inline Cap cap_mk_pmp(uint64_t addr, uint64_t rwx) {
        ASSERT((addr & 0xFFFFFFFFull) == addr);
        ASSERT(rwx > 0 && rwx < 8);
        return __mk_cap(CAP_PMP, addr | rwx << 32, 0);
}

static inline uint64_t cap_pmp_addr(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_PMP);
        return cap.word1 & 0xFFFFFFFF;
}

static inline uint64_t cap_pmp_rwx(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_PMP);
        return (cap.word1 >> 32) & 0x7;
}

static inline Cap cap_mk_pmp_hidden(uint64_t addr, uint64_t rwx) {
        ASSERT((addr & 0xFFFFFFFFFFFFFFull) == addr);
        ASSERT(rwx > 0 && rwx < 8);
        uint64_t cfg = 0x18 | rwx;
        return __mk_cap(CAP_PMP_HIDDEN, 0, addr << 8 | cfg);
}

static inline uint64_t cap_pmp_hidden_addr(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_PMP_HIDDEN);
        return cap.word1 >> 8;
}

static inline uint64_t cap_pmp_hidden_cfg(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_PMP_HIDDEN);
        return cap.word1 & 0xFF;
}

static inline Cap cap_mk_time(uint64_t core, uint64_t begin, uint64_t end,
                              uint64_t free, uint64_t id, uint64_t id_end,
                              uint64_t id_free) {
        ASSERT((core & 0xFF) == core);
        ASSERT((begin & 0xFFFF) == begin);
        ASSERT((end & 0xFFFF) == end);
        ASSERT((free & 0xFFFF) == free);
        ASSERT((id & 0xFF) == id);
        ASSERT((id_free & 0xFF) == id_free);
        ASSERT((id_end & 0xFF) == id_end);
        ASSERT(begin <= free);
        ASSERT(free <= end);
        ASSERT(id < id_free);
        ASSERT(id_free <= id_end);

        return __mk_cap(CAP_TIME, core << 8 | begin << 32 | end << 16 | free,
                        id | id_end << 8 | id_free << 16);
}

static inline uint64_t cap_time_core(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_TIME);
        return (cap.word0 >> 48) & 0xFF;
}

static inline uint64_t cap_time_begin(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_TIME);
        return (cap.word0 >> 32) & 0xFFFF;
}

static inline uint64_t cap_time_end(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_TIME);
        return (cap.word0 >> 16) & 0xFFFF;
}

static inline uint64_t cap_time_free(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_TIME);
        return (cap.word0) & 0xFFFF;
}

static inline uint64_t cap_time_id(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_TIME);
        return (cap.word1) & 0xFF;
}

static inline uint64_t cap_time_id_free(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_TIME);
        return (cap.word1 >> 16) & 0xFF;
}

static inline uint64_t cap_time_id_end(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_TIME);
        return (cap.word1 >> 8) & 0xFF;
}

static inline Cap cap_mk_channels(uint64_t begin, uint64_t end, uint64_t free,
                                  uint64_t ep) {
        ASSERT((begin & 0xFFFF) == begin);
        ASSERT((end & 0xFFFF) == end);
        ASSERT((free & 0xFFFF) == free);
        ASSERT(begin <= end);
        ASSERT(begin <= free);
        ASSERT(free <= end);
        return __mk_cap(CAP_CHANNELS, ep << 48 | begin << 32 | end << 16 | free,
                        0);
}

static inline uint64_t cap_channels_begin(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_CHANNELS);
        return (cap.word0 >> 16) & 0xFFFF;
}

static inline uint64_t cap_channels_end(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_CHANNELS);
        return (cap.word0 >> 32) & 0xFFFF;
}

static inline uint64_t cap_channels_free(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_CHANNELS);
        return (cap.word0 >> 32) & 0xFFFF;
}

static inline uint64_t cap_channels_ep(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_CHANNELS);
        return (cap.word0 >> 48) & 0xFF;
}

static inline Cap cap_mk_endpoint(uint64_t channel, uint64_t receive,
                                  uint64_t send, uint64_t grant) {
        ASSERT((channel & 0xFFFF) == channel);
        ASSERT((receive & 0xFF) == receive);
        ASSERT((send & 0xFF) == send);
        ASSERT((grant & 0xFF) == grant);
        ASSERT(!grant || send);
        return __mk_cap(CAP_ENDPOINT,
                        receive << 32 | send << 24 | grant << 16 | channel, 0);
}

static inline uint64_t cap_endpoint_channel(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_ENDPOINT);
        return (cap.word0) & 0xFFFF;
}

static inline uint64_t cap_endpoint_receive(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_ENDPOINT);
        return (cap.word0 >> 32) & 0xFF;
}

static inline uint64_t cap_endpoint_send(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_ENDPOINT);
        return (cap.word0 >> 24) & 0xFF;
}

static inline uint64_t cap_endpoint_grant(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_ENDPOINT);
        return (cap.word0 >> 16) & 0xFF;
}

static inline Cap cap_mk_supervisor(uint64_t begin, uint64_t end,
                                    uint64_t free) {
        ASSERT((begin & 0x7F) == begin);
        ASSERT((end & 0x7F) == end);
        ASSERT((free & 0x7F) == free);
        ASSERT(begin <= end);
        ASSERT(begin <= free);
        ASSERT(free <= end);
        return __mk_cap(CAP_SUPERVISOR, begin << 16 | end << 8 | free, 0);
}

static inline uint64_t cap_supervisor_begin(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_SUPERVISOR);
        return (cap.word0 >> 16) & 0x7F;
}

static inline uint64_t cap_supervisor_end(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_SUPERVISOR);
        return (cap.word0 >> 8) & 0x7F;
}

static inline uint64_t cap_supervisor_free(const Cap cap) {
        ASSERT(cap_get_type(cap) == CAP_SUPERVISOR);
        return (cap.word0) & 0x7F;
}

static inline bool cap_is_child_ms_ms(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_MEMORY);
        ASSERT(cap_get_type(child) == CAP_MEMORY);
        return cap_memory_begin(parent) <= cap_memory_begin(child) &&
               cap_memory_end(child) <= cap_memory_end(parent);
}

static inline bool cap_is_child_ms_pe(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_MEMORY);
        ASSERT(cap_get_type(child) == CAP_PMP);
        uint64_t child_begin, child_end;
        uint64_t addr = cap_pmp_addr(child);
        pmp_napot_bounds(addr, &child_begin, &child_end);
        child_begin >>= 10;
        child_end >>= 10;
        uint64_t parent_rwx = cap_memory_rwx(parent);
        uint64_t child_rwx = cap_pmp_rwx(child);
        return cap_memory_begin(parent) <= child_begin &&
               child_end <= cap_memory_end(parent) &&
               (child_rwx & parent_rwx) == child_rwx;
}

static inline bool cap_is_child_ms(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_MEMORY);
        switch (cap_get_type(child)) {
                case CAP_MEMORY:
                        return cap_is_child_ms_ms(parent, child);
                case CAP_PMP:
                        return cap_is_child_ms_pe(parent, child);
                case CAP_PMP_HIDDEN:
                        return true;
                default:
                        return false;
        }
}

static inline bool cap_is_child_ts_ts(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_TIME);
        ASSERT(cap_get_type(child) == CAP_TIME);
        return cap_time_core(parent) == cap_time_core(child) &&
               cap_time_begin(parent) <= cap_time_begin(child) &&
               cap_time_end(child) <= cap_time_end(child) &&
               cap_time_id(parent) < cap_time_id(child) &&
               cap_time_id_end(child) <= cap_time_id_end(parent);
}

static inline bool cap_is_child_ts(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_TIME);
        switch (cap_get_type(child)) {
                case CAP_TIME:
                        return cap_is_child_ts_ts(parent, child);
                default:
                        return false;
        }
}

static inline bool cap_is_child_pe(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_PMP);
        return cap_get_type(child) == CAP_PMP_HIDDEN;
}

static inline bool cap_is_child_ch_ch(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_CHANNELS);
        ASSERT(cap_get_type(child) == CAP_CHANNELS);
        return cap_channels_begin(parent) <= cap_channels_begin(child) &&
               cap_channels_end(child) <= cap_channels_end(parent);
}

static inline bool cap_is_child_ch_ep(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_CHANNELS);
        ASSERT(cap_get_type(child) == CAP_ENDPOINT);
        return cap_channels_begin(parent) <= cap_endpoint_channel(child) &&
               cap_endpoint_channel(child) < cap_channels_end(parent);
}

static inline bool cap_is_child_ch(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_CHANNELS);
        switch (cap_get_type(child)) {
                case CAP_CHANNELS:
                        return cap_is_child_ch_ch(parent, child);
                case CAP_ENDPOINT:
                        return cap_is_child_ch_ep(parent, child);
                default:
                        return false;
        }
}

static inline bool cap_is_child(const Cap parent, const Cap child) {
        switch (cap_get_type(parent)) {
                case CAP_MEMORY:
                        return cap_is_child_ms(parent, child);
                case CAP_TIME:
                        return cap_is_child_ts(parent, child);
                case CAP_PMP:
                        return cap_is_child_pe(parent, child);
                case CAP_CHANNELS:
                        return cap_is_child_ch(parent, child);
                default:
                        return false;
        }
}

static inline bool cap_can_derive_ms_ms(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_MEMORY);
        ASSERT(cap_get_type(child) == CAP_MEMORY);
        if (cap_memory_pmp(parent))
                return false;
        uint64_t parent_end = cap_memory_end(parent);
        uint64_t parent_free = cap_memory_free(parent);
        uint64_t parent_rwx = cap_memory_rwx(parent);
        uint64_t child_begin = cap_memory_begin(child);
        uint64_t child_end = cap_memory_end(child);
        uint64_t child_free = cap_memory_free(child);
        uint64_t child_rwx = cap_memory_rwx(child);
        return (child_free == child_begin) && child_begin <= child_end &&
               parent_free <= child_begin && child_end <= parent_end &&
               (child_rwx & parent_rwx) == child_rwx;
}

static inline bool cap_can_derive_ms_pe(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_MEMORY);
        ASSERT(cap_get_type(child) == CAP_PMP);
        uint64_t parent_end = cap_memory_end(parent);
        uint64_t parent_free = cap_memory_free(parent);
        uint64_t parent_rwx = cap_memory_rwx(parent);
        uint64_t child_addr = cap_pmp_addr(child);
        uint64_t child_rwx = cap_pmp_rwx(child);
        uint64_t child_begin, child_end;
        pmp_napot_bounds(child_addr, &child_begin, &child_end);
        return parent_free <= child_begin && child_end <= parent_end &&
               (child_rwx & parent_rwx) == child_rwx;
}

static inline bool cap_can_derive_ts_ts(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_TIME);
        ASSERT(cap_get_type(child) == CAP_TIME);
        uint64_t parent_core = cap_time_core(parent);
        uint64_t parent_free = cap_time_free(parent);
        uint64_t parent_end = cap_time_end(parent);
        uint64_t parent_id_free = cap_time_id_free(parent);
        uint64_t parent_id_end = cap_time_id_end(parent);
        uint64_t child_core = cap_time_core(child);
        uint64_t child_begin = cap_time_begin(child);
        uint64_t child_free = cap_time_free(child);
        uint64_t child_end = cap_time_end(child);
        uint64_t child_id = cap_time_id(child);
        uint64_t child_id_free = cap_time_id_free(child);
        uint64_t child_id_end = cap_time_id_end(child);
        return (parent_core == child_core) && child_begin == child_free &&
               (child_id + 1) == child_id_free && child_begin <= child_end &&
               child_id_free <= child_id_end && parent_free <= child_begin &&
               child_end <= parent_end && parent_id_free <= child_id &&
               child_id_end <= parent_id_end;
}

static inline bool cap_can_derive_ch_ch(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_CHANNELS);
        ASSERT(cap_get_type(child) == CAP_CHANNELS);
        if (cap_channels_ep(parent))
                return false;
        uint64_t parent_free = cap_channels_free(parent);
        uint64_t parent_end = cap_channels_end(parent);
        uint64_t child_begin = cap_channels_begin(child);
        uint64_t child_free = cap_channels_free(child);
        uint64_t child_end = cap_channels_end(child);
        return (child_begin == child_free) && child_begin <= child_end &&
               parent_free <= child_begin && child_end <= parent_end;
}

static inline bool cap_can_derive_ch_ep(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_CHANNELS);
        ASSERT(cap_get_type(child) == CAP_CHANNELS);
        uint64_t parent_free = cap_channels_free(parent);
        uint64_t parent_end = cap_channels_end(parent);
        uint64_t child_channel = cap_endpoint_channel(child);
        return parent_free <= child_channel && child_channel < parent_end;
}

static inline bool cap_can_derive_su_su(const Cap parent, const Cap child) {
        ASSERT(cap_get_type(parent) == CAP_SUPERVISOR);
        ASSERT(cap_get_type(child) == CAP_SUPERVISOR);
        uint64_t parent_free = cap_supervisor_free(parent);
        uint64_t parent_end = cap_supervisor_end(parent);
        uint64_t child_begin = cap_supervisor_begin(child);
        uint64_t child_free = cap_supervisor_free(child);
        uint64_t child_end = cap_supervisor_end(child);
        return child_begin == child_free && parent_free <= child_begin &&

               child_end <= parent_end;
}

static inline void cap_memory_set_free(Cap *cap, uint64_t free) {
        ASSERT(cap_get_type(*cap) == CAP_MEMORY);
        ASSERT((free & 0xFFFFFFFFull) == free);
        cap->word0 = (cap->word0 & ~0xFFFFFFFFull) | free;
}

static inline void cap_memory_set_pmp(Cap *cap, uint64_t pmp) {
        ASSERT(cap_get_type(*cap) == CAP_MEMORY);
        ASSERT((pmp & 1) == pmp);
        cap->word0 = (cap->word0 & ~0xFF0000000000ull) | pmp << 40;
}

static inline void cap_time_set_free(Cap *cap, uint64_t free) {
        ASSERT(cap_get_type(*cap) == CAP_TIME);
        ASSERT((free & 0xFFFF) == free);
        cap->word0 = (cap->word0 & ~0xFFFF) | free;
}

static inline void cap_time_set_id_free(Cap *cap, uint64_t id_free) {
        ASSERT(cap_get_type(*cap) == CAP_TIME);
        ASSERT((id_free & 0xFF) == id_free);
        cap->word1 = (cap->word1 & ~0xFF0000) | id_free << 16;
}

static inline void cap_channels_set_free(Cap *cap, uint64_t free) {
        ASSERT(cap_get_type(*cap) == CAP_CHANNELS);
        ASSERT((free & 0xFFFF) == free);
        cap->word0 = (cap->word0 & ~0xFFFFull) | free;
}

static inline void cap_channels_set_ep(Cap *cap, uint64_t ep) {
        ASSERT(cap_get_type(*cap) == CAP_CHANNELS);
        ASSERT((ep & 1) == ep);
        cap->word0 = (cap->word0 & ~0xFF000000000000ull) | ep << 48;
}

static inline void cap_supervisor_set_free(Cap *cap, uint64_t free) {
        ASSERT(cap_get_type(*cap) == CAP_SUPERVISOR);
        ASSERT((free & 0xFF) == free);
        cap->word0 = (cap->word0 & ~0xFFull) | free;
}
