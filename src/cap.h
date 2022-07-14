#pragma once
#include "config.h"
#include "kassert.h"
#include "pmp.h"

#define NULL_CAP ((struct cap){0, 0})

enum cap_type {
        CAP_INVALID,
        CAP_MEMORY,
        CAP_PMP,
        CAP_TIME,
        CAP_CHANNELS,
        CAP_ENDPOINT,
        CAP_SUPERVISOR
};

struct cap {
        unsigned long long word0, word1;
};

static inline enum cap_type cap_get_type(struct cap cap) {
        return (cap.word0 >> 56) & 0xff;
}

static inline struct cap cap_mk_memory(uint64_t begin, uint64_t end,
                                       uint64_t rwx, uint64_t free,
                                       uint64_t pmp) {
        kassert((begin & 0xffffffffull) == begin);
        kassert((end & 0xffffffffull) == end);
        kassert((rwx & 0xffull) == rwx);
        kassert((free & 0xffffffffull) == free);
        kassert((pmp & 0xffull) == pmp);
        kassert(begin == free);
        kassert(begin < end);
        kassert(pmp == 0);
        kassert(rwx == 0x4 || rwx == 0x5 || rwx == 0x6 || rwx == 0x7);
        struct cap c;
        c.word0 = (uint64_t)CAP_MEMORY << 56;
        c.word1 = 0;
        c.word0 |= pmp;
        c.word0 |= rwx << 8;
        c.word0 |= begin << 16;
        c.word1 |= free;
        c.word1 |= end << 32;
        return c;
}
static inline uint64_t cap_memory_get_begin(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_MEMORY);
        return (cap.word0 >> 16) & 0xffffffffull;
}
static inline void cap_memory_set_begin(struct cap *cap, uint64_t begin) {
        kassert((cap->word0 >> 56) == CAP_MEMORY);
        kassert((begin & 0xffffffffull) == begin);
        cap->word0 = (cap->word0 & ~0xffffffff0000ull) | begin << 16;
}
static inline uint64_t cap_memory_get_end(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_MEMORY);
        return (cap.word1 >> 32) & 0xffffffffull;
}
static inline void cap_memory_set_end(struct cap *cap, uint64_t end) {
        kassert((cap->word0 >> 56) == CAP_MEMORY);
        kassert((end & 0xffffffffull) == end);
        cap->word1 = (cap->word1 & ~0xffffffff00000000ull) | end << 32;
}
static inline uint64_t cap_memory_get_rwx(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_MEMORY);
        return (cap.word0 >> 8) & 0xffull;
}
static inline void cap_memory_set_rwx(struct cap *cap, uint64_t rwx) {
        kassert((cap->word0 >> 56) == CAP_MEMORY);
        kassert((rwx & 0xffull) == rwx);
        cap->word0 = (cap->word0 & ~0xff00ull) | rwx << 8;
}
static inline uint64_t cap_memory_get_free(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_MEMORY);
        return cap.word1 & 0xffffffffull;
}
static inline void cap_memory_set_free(struct cap *cap, uint64_t free) {
        kassert((cap->word0 >> 56) == CAP_MEMORY);
        kassert((free & 0xffffffffull) == free);
        cap->word1 = (cap->word1 & ~0xffffffffull) | free;
}
static inline uint64_t cap_memory_get_pmp(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_MEMORY);
        return cap.word0 & 0xffull;
}
static inline void cap_memory_set_pmp(struct cap *cap, uint64_t pmp) {
        kassert((cap->word0 >> 56) == CAP_MEMORY);
        kassert((pmp & 0xffull) == pmp);
        cap->word0 = (cap->word0 & ~0xffull) | pmp;
}
static inline struct cap cap_mk_pmp(uint64_t addr, uint64_t rwx) {
        kassert((addr & 0xffffffffull) == addr);
        kassert((rwx & 0xffull) == rwx);
        kassert(rwx == 0x4 || rwx == 0x5 || rwx == 0x6 || rwx == 0x7);
        struct cap c;
        c.word0 = (uint64_t)CAP_PMP << 56;
        c.word1 = 0;
        c.word0 |= rwx;
        c.word0 |= addr << 8;
        return c;
}
static inline uint64_t cap_pmp_get_addr(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_PMP);
        return (cap.word0 >> 8) & 0xffffffffull;
}
static inline void cap_pmp_set_addr(struct cap *cap, uint64_t addr) {
        kassert((cap->word0 >> 56) == CAP_PMP);
        kassert((addr & 0xffffffffull) == addr);
        cap->word0 = (cap->word0 & ~0xffffffff00ull) | addr << 8;
}
static inline uint64_t cap_pmp_get_rwx(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_PMP);
        return cap.word0 & 0xffull;
}
static inline void cap_pmp_set_rwx(struct cap *cap, uint64_t rwx) {
        kassert((cap->word0 >> 56) == CAP_PMP);
        kassert((rwx & 0xffull) == rwx);
        cap->word0 = (cap->word0 & ~0xffull) | rwx;
}
static inline struct cap cap_mk_time(uint64_t hartid, uint64_t pid,
                                     uint64_t begin, uint64_t end,
                                     uint64_t free, uint64_t depth) {
        kassert((hartid & 0xffull) == hartid);
        kassert((pid & 0xffull) == pid);
        kassert((begin & 0xffffull) == begin);
        kassert((end & 0xffffull) == end);
        kassert((free & 0xffffull) == free);
        kassert((depth & 0xffull) == depth);
        kassert(MIN_HARTID <= hartid);
        kassert(hartid <= MAX_HARTID);
        kassert(begin == free);
        kassert(begin < end);
        kassert(end <= N_QUANTUM);
        struct cap c;
        c.word0 = (uint64_t)CAP_TIME << 56;
        c.word1 = 0;
        c.word0 |= depth;
        c.word0 |= end << 8;
        c.word0 |= begin << 24;
        c.word0 |= pid << 40;
        c.word0 |= hartid << 48;
        c.word1 |= free;
        return c;
}
static inline uint64_t cap_time_get_hartid(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_TIME);
        return (cap.word0 >> 48) & 0xffull;
}
static inline void cap_time_set_hartid(struct cap *cap, uint64_t hartid) {
        kassert((cap->word0 >> 56) == CAP_TIME);
        kassert((hartid & 0xffull) == hartid);
        cap->word0 = (cap->word0 & ~0xff000000000000ull) | hartid << 48;
}
static inline uint64_t cap_time_get_pid(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_TIME);
        return (cap.word0 >> 40) & 0xffull;
}
static inline void cap_time_set_pid(struct cap *cap, uint64_t pid) {
        kassert((cap->word0 >> 56) == CAP_TIME);
        kassert((pid & 0xffull) == pid);
        cap->word0 = (cap->word0 & ~0xff0000000000ull) | pid << 40;
}
static inline uint64_t cap_time_get_begin(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_TIME);
        return (cap.word0 >> 24) & 0xffffull;
}
static inline void cap_time_set_begin(struct cap *cap, uint64_t begin) {
        kassert((cap->word0 >> 56) == CAP_TIME);
        kassert((begin & 0xffffull) == begin);
        cap->word0 = (cap->word0 & ~0xffff000000ull) | begin << 24;
}
static inline uint64_t cap_time_get_end(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_TIME);
        return (cap.word0 >> 8) & 0xffffull;
}
static inline void cap_time_set_end(struct cap *cap, uint64_t end) {
        kassert((cap->word0 >> 56) == CAP_TIME);
        kassert((end & 0xffffull) == end);
        cap->word0 = (cap->word0 & ~0xffff00ull) | end << 8;
}
static inline uint64_t cap_time_get_free(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_TIME);
        return cap.word1 & 0xffffull;
}
static inline void cap_time_set_free(struct cap *cap, uint64_t free) {
        kassert((cap->word0 >> 56) == CAP_TIME);
        kassert((free & 0xffffull) == free);
        cap->word1 = (cap->word1 & ~0xffffull) | free;
}
static inline uint64_t cap_time_get_depth(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_TIME);
        return cap.word0 & 0xffull;
}
static inline void cap_time_set_depth(struct cap *cap, uint64_t depth) {
        kassert((cap->word0 >> 56) == CAP_TIME);
        kassert((depth & 0xffull) == depth);
        cap->word0 = (cap->word0 & ~0xffull) | depth;
}
static inline struct cap cap_mk_channels(uint64_t begin, uint64_t end,
                                         uint64_t free, uint64_t ep) {
        kassert((begin & 0xffffull) == begin);
        kassert((end & 0xffffull) == end);
        kassert((free & 0xffffull) == free);
        kassert((ep & 0xffull) == ep);
        kassert(begin == free);
        kassert(begin < end);
        kassert(end <= N_CHANNELS);
        kassert(ep == 0);
        struct cap c;
        c.word0 = (uint64_t)CAP_CHANNELS << 56;
        c.word1 = 0;
        c.word0 |= ep;
        c.word0 |= free << 8;
        c.word0 |= end << 24;
        c.word0 |= begin << 40;
        return c;
}
static inline uint64_t cap_channels_get_begin(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_CHANNELS);
        return (cap.word0 >> 40) & 0xffffull;
}
static inline void cap_channels_set_begin(struct cap *cap, uint64_t begin) {
        kassert((cap->word0 >> 56) == CAP_CHANNELS);
        kassert((begin & 0xffffull) == begin);
        cap->word0 = (cap->word0 & ~0xffff0000000000ull) | begin << 40;
}
static inline uint64_t cap_channels_get_end(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_CHANNELS);
        return (cap.word0 >> 24) & 0xffffull;
}
static inline void cap_channels_set_end(struct cap *cap, uint64_t end) {
        kassert((cap->word0 >> 56) == CAP_CHANNELS);
        kassert((end & 0xffffull) == end);
        cap->word0 = (cap->word0 & ~0xffff000000ull) | end << 24;
}
static inline uint64_t cap_channels_get_free(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_CHANNELS);
        return (cap.word0 >> 8) & 0xffffull;
}
static inline void cap_channels_set_free(struct cap *cap, uint64_t free) {
        kassert((cap->word0 >> 56) == CAP_CHANNELS);
        kassert((free & 0xffffull) == free);
        cap->word0 = (cap->word0 & ~0xffff00ull) | free << 8;
}
static inline uint64_t cap_channels_get_ep(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_CHANNELS);
        return cap.word0 & 0xffull;
}
static inline void cap_channels_set_ep(struct cap *cap, uint64_t ep) {
        kassert((cap->word0 >> 56) == CAP_CHANNELS);
        kassert((ep & 0xffull) == ep);
        cap->word0 = (cap->word0 & ~0xffull) | ep;
}
static inline struct cap cap_mk_endpoint(uint64_t channel, uint64_t mode) {
        kassert((channel & 0xffffull) == channel);
        kassert((mode & 0xffull) == mode);
        kassert(mode <= 7);
        kassert(channel < N_CHANNELS);
        struct cap c;
        c.word0 = (uint64_t)CAP_ENDPOINT << 56;
        c.word1 = 0;
        c.word0 |= mode;
        c.word0 |= channel << 8;
        return c;
}
static inline uint64_t cap_endpoint_get_channel(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_ENDPOINT);
        return (cap.word0 >> 8) & 0xffffull;
}
static inline void cap_endpoint_set_channel(struct cap *cap, uint64_t channel) {
        kassert((cap->word0 >> 56) == CAP_ENDPOINT);
        kassert((channel & 0xffffull) == channel);
        cap->word0 = (cap->word0 & ~0xffff00ull) | channel << 8;
}
static inline uint64_t cap_endpoint_get_mode(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_ENDPOINT);
        return cap.word0 & 0xffull;
}
static inline void cap_endpoint_set_mode(struct cap *cap, uint64_t mode) {
        kassert((cap->word0 >> 56) == CAP_ENDPOINT);
        kassert((mode & 0xffull) == mode);
        cap->word0 = (cap->word0 & ~0xffull) | mode;
}
static inline struct cap cap_mk_supervisor(uint64_t begin, uint64_t end,
                                           uint64_t free) {
        kassert((begin & 0xffull) == begin);
        kassert((end & 0xffull) == end);
        kassert((free & 0xffull) == free);
        kassert(begin == free);
        kassert(begin < end);
        kassert(end <= N_PROC);
        struct cap c;
        c.word0 = (uint64_t)CAP_SUPERVISOR << 56;
        c.word1 = 0;
        c.word0 |= free;
        c.word0 |= end << 8;
        c.word0 |= begin << 16;
        return c;
}
static inline uint64_t cap_supervisor_get_begin(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_SUPERVISOR);
        return (cap.word0 >> 16) & 0xffull;
}
static inline void cap_supervisor_set_begin(struct cap *cap, uint64_t begin) {
        kassert((cap->word0 >> 56) == CAP_SUPERVISOR);
        kassert((begin & 0xffull) == begin);
        cap->word0 = (cap->word0 & ~0xff0000ull) | begin << 16;
}
static inline uint64_t cap_supervisor_get_end(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_SUPERVISOR);
        return (cap.word0 >> 8) & 0xffull;
}
static inline void cap_supervisor_set_end(struct cap *cap, uint64_t end) {
        kassert((cap->word0 >> 56) == CAP_SUPERVISOR);
        kassert((end & 0xffull) == end);
        cap->word0 = (cap->word0 & ~0xff00ull) | end << 8;
}
static inline uint64_t cap_supervisor_get_free(struct cap cap) {
        kassert((cap.word0 >> 56) == CAP_SUPERVISOR);
        return cap.word0 & 0xffull;
}
static inline void cap_supervisor_set_free(struct cap *cap, uint64_t free) {
        kassert((cap->word0 >> 56) == CAP_SUPERVISOR);
        kassert((free & 0xffull) == free);
        cap->word0 = (cap->word0 & ~0xffull) | free;
}
static inline int cap_is_child(struct cap p, struct cap c) {
        int b = 1;
        enum cap_type parent_type = cap_get_type(p);
        enum cap_type child_type = cap_get_type(c);
        if (parent_type == CAP_MEMORY && child_type == CAP_MEMORY) {
                b &= cap_memory_get_begin(p) <= cap_memory_get_begin(c);
                b &= cap_memory_get_end(c) <= cap_memory_get_end(p);
                b &= (cap_memory_get_rwx(c) & cap_memory_get_rwx(p)) ==
                     cap_memory_get_rwx(c);
                return b;
        }
        if (parent_type == CAP_MEMORY && child_type == CAP_PMP) {
                b &= cap_memory_get_begin(p) <=
                     pmp_napot_begin(cap_pmp_get_addr(c));
                b &=
                    pmp_napot_end(cap_pmp_get_addr(c)) <= cap_memory_get_end(p);
                b &= (cap_pmp_get_rwx(c) & cap_memory_get_rwx(p)) ==
                     cap_pmp_get_rwx(c);
                return b;
        }
        if (parent_type == CAP_TIME && child_type == CAP_TIME) {
                b &= cap_time_get_begin(p) <= cap_time_get_begin(c);
                b &= cap_time_get_end(c) <= cap_time_get_end(p);
                b &= cap_time_get_hartid(p) == cap_time_get_hartid(c);
                b &= cap_time_get_depth(p) < cap_time_get_depth(c);
                return b;
        }
        if (parent_type == CAP_CHANNELS && child_type == CAP_CHANNELS) {
                b &= cap_channels_get_begin(p) <= cap_channels_get_begin(c);
                b &= cap_channels_get_end(c) <= cap_channels_get_end(p);
                return b;
        }
        if (parent_type == CAP_CHANNELS && child_type == CAP_ENDPOINT) {
                b &= cap_channels_get_begin(p) <= cap_endpoint_get_channel(c);
                b &= cap_endpoint_get_channel(c) < cap_channels_get_end(p);
                return b;
        }
        if (parent_type == CAP_SUPERVISOR && child_type == CAP_SUPERVISOR) {
                b &= cap_supervisor_get_begin(p) <= cap_supervisor_get_begin(c);
                b &= cap_supervisor_get_end(c) <= cap_supervisor_get_end(p);
                return b;
        }
        return 0;
}
static inline int cap_can_derive(struct cap p, struct cap c) {
        int b = 1;
        enum cap_type parent_type = cap_get_type(p);
        enum cap_type child_type = cap_get_type(c);
        if (parent_type == CAP_MEMORY && child_type == CAP_MEMORY) {
                b &= cap_memory_get_pmp(p) == 0;
                b &= cap_memory_get_pmp(c) == 0;
                b &= cap_memory_get_free(p) <= cap_memory_get_begin(c);
                b &= cap_memory_get_end(c) <= cap_memory_get_end(p);
                b &= (cap_memory_get_rwx(c) & cap_memory_get_rwx(p)) ==
                     cap_memory_get_rwx(c);
                b &= cap_memory_get_free(c) == cap_memory_get_begin(c);
                b &= cap_memory_get_begin(c) < cap_memory_get_end(c);
                return b;
        }
        if (parent_type == CAP_MEMORY && child_type == CAP_PMP) {
                b &= cap_memory_get_free(p) <=
                     pmp_napot_begin(cap_pmp_get_addr(c));
                b &=
                    pmp_napot_end(cap_pmp_get_addr(c)) <= cap_memory_get_end(p);
                b &= (cap_pmp_get_rwx(c) & cap_memory_get_rwx(p)) ==
                     cap_pmp_get_rwx(c);
                return b;
        }
        if (parent_type == CAP_TIME && child_type == CAP_TIME) {
                b &= cap_time_get_free(p) <= cap_time_get_begin(c);
                b &= cap_time_get_end(c) <= cap_time_get_end(p);
                b &= cap_time_get_hartid(p) == cap_time_get_hartid(c);
                b &= cap_time_get_depth(p) < cap_time_get_depth(c);
                b &= cap_time_get_free(c) == cap_time_get_begin(c);
                b &= cap_time_get_begin(c) < cap_time_get_end(c);
                return b;
        }
        if (parent_type == CAP_CHANNELS && child_type == CAP_CHANNELS) {
                b &= cap_channels_get_ep(p) == 0;
                b &= cap_channels_get_ep(c) == 0;
                b &= cap_channels_get_free(p) <= cap_channels_get_begin(c);
                b &= cap_channels_get_end(c) <= cap_channels_get_end(p);
                b &= cap_channels_get_free(c) == cap_channels_get_begin(c);
                b &= cap_channels_get_begin(c) < cap_channels_get_end(c);
                return b;
        }
        if (parent_type == CAP_CHANNELS && child_type == CAP_ENDPOINT) {
                b &= cap_channels_get_free(p) <= cap_endpoint_get_channel(c);
                b &= cap_endpoint_get_channel(c) < cap_channels_get_end(p);
                b &= cap_endpoint_get_mode(c) <= 7;
                return b;
        }
        if (parent_type == CAP_SUPERVISOR && child_type == CAP_SUPERVISOR) {
                b &= cap_supervisor_get_free(p) <= cap_supervisor_get_begin(c);
                b &= cap_supervisor_get_end(c) <= cap_supervisor_get_end(p);
                b &= cap_supervisor_get_free(c) == cap_supervisor_get_begin(c);
                b &= cap_supervisor_get_begin(c) < cap_supervisor_get_end(c);
                return b;
        }
        return 0;
}
