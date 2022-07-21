#pragma once
#include "config.h"
#include "kassert.h"
#include "pmp.h"

#define NULL_CAP ((cap_t){0, 0})

typedef enum cap_type cap_type_t;
typedef struct cap cap_t;

enum cap_type {
        CAP_TYPE_EMPTY,
        CAP_TYPE_MEMORY,
        CAP_TYPE_PMP,
        CAP_TYPE_TIME,
        CAP_TYPE_CHANNELS,
        CAP_TYPE_RECEIVER,
        CAP_TYPE_SENDER,
        CAP_TYPE_SUPERVISOR
};

struct cap {
        uint64_t word0, word1;
};

static inline cap_type_t cap_get_type(cap_t cap)
{
        return (cap.word0 >> 56) & 0xff;
}

static inline cap_t cap_mk_memory(uint64_t begin, uint64_t end, uint64_t rwx, uint64_t free, uint64_t pmp)
{
        kassert((begin & 0xffffffffull) == begin);
        kassert((end & 0xffffffffull) == end);
        kassert((rwx & 0xffull) == rwx);
        kassert((free & 0xffffffffull) == free);
        kassert((pmp & 0xffull) == pmp);
        kassert(begin == free);
        kassert(begin < end);
        kassert(pmp == 0);
        kassert(rwx == 0x4 || rwx == 0x5 || rwx == 0x6 || rwx == 0x7);
        cap_t c;
        c.word0 = (uint64_t)CAP_TYPE_MEMORY << 56;
        c.word1 = 0;
        c.word0 |= pmp;
        c.word0 |= rwx << 8;
        c.word0 |= begin << 16;
        c.word1 |= free;
        c.word1 |= end << 32;
        return c;
}

static inline uint64_t cap_memory_get_begin(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_MEMORY);
        return (cap.word0 >> 16) & 0xffffffffull;
}

static inline void cap_memory_set_begin(cap_t* cap, uint64_t begin)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_MEMORY);
        kassert((begin & 0xffffffffull) == begin);
        cap->word0 = (cap->word0 & ~0xffffffff0000ull) | begin << 16;
}

static inline uint64_t cap_memory_get_end(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_MEMORY);
        return (cap.word1 >> 32) & 0xffffffffull;
}

static inline void cap_memory_set_end(cap_t* cap, uint64_t end)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_MEMORY);
        kassert((end & 0xffffffffull) == end);
        cap->word1 = (cap->word1 & ~0xffffffff00000000ull) | end << 32;
}

static inline uint64_t cap_memory_get_rwx(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_MEMORY);
        return (cap.word0 >> 8) & 0xffull;
}

static inline void cap_memory_set_rwx(cap_t* cap, uint64_t rwx)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_MEMORY);
        kassert((rwx & 0xffull) == rwx);
        cap->word0 = (cap->word0 & ~0xff00ull) | rwx << 8;
}

static inline uint64_t cap_memory_get_free(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_MEMORY);
        return cap.word1 & 0xffffffffull;
}

static inline void cap_memory_set_free(cap_t* cap, uint64_t free)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_MEMORY);
        kassert((free & 0xffffffffull) == free);
        cap->word1 = (cap->word1 & ~0xffffffffull) | free;
}

static inline uint64_t cap_memory_get_pmp(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_MEMORY);
        return cap.word0 & 0xffull;
}

static inline void cap_memory_set_pmp(cap_t* cap, uint64_t pmp)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_MEMORY);
        kassert((pmp & 0xffull) == pmp);
        cap->word0 = (cap->word0 & ~0xffull) | pmp;
}

static inline cap_t cap_mk_pmp(uint64_t addr, uint64_t rwx)
{
        kassert((addr & 0xffffffffull) == addr);
        kassert((rwx & 0xffull) == rwx);
        kassert(rwx == 0x4 || rwx == 0x5 || rwx == 0x6 || rwx == 0x7);
        cap_t c;
        c.word0 = (uint64_t)CAP_TYPE_PMP << 56;
        c.word1 = 0;
        c.word0 |= rwx;
        c.word0 |= addr << 8;
        return c;
}

static inline uint64_t cap_pmp_get_addr(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_PMP);
        return (cap.word0 >> 8) & 0xffffffffull;
}

static inline void cap_pmp_set_addr(cap_t* cap, uint64_t addr)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_PMP);
        kassert((addr & 0xffffffffull) == addr);
        cap->word0 = (cap->word0 & ~0xffffffff00ull) | addr << 8;
}

static inline uint64_t cap_pmp_get_rwx(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_PMP);
        return cap.word0 & 0xffull;
}

static inline void cap_pmp_set_rwx(cap_t* cap, uint64_t rwx)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_PMP);
        kassert((rwx & 0xffull) == rwx);
        cap->word0 = (cap->word0 & ~0xffull) | rwx;
}

static inline cap_t cap_mk_time(uint64_t hartid, uint64_t begin, uint64_t end, uint64_t free, uint64_t depth)
{
        kassert((hartid & 0xffull) == hartid);
        kassert((begin & 0xffffull) == begin);
        kassert((end & 0xffffull) == end);
        kassert((free & 0xffffull) == free);
        kassert((depth & 0xffull) == depth);
        kassert(MIN_HARTID <= hartid);
        kassert(hartid <= MAX_HARTID);
        kassert(begin == free);
        kassert(begin < end);
        kassert(end <= N_QUANTUM);
        cap_t c;
        c.word0 = (uint64_t)CAP_TYPE_TIME << 56;
        c.word1 = 0;
        c.word0 |= free;
        c.word0 |= end << 16;
        c.word0 |= begin << 32;
        c.word0 |= hartid << 48;
        c.word1 |= depth;
        return c;
}

static inline uint64_t cap_time_get_hartid(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_TIME);
        return (cap.word0 >> 48) & 0xffull;
}

static inline void cap_time_set_hartid(cap_t* cap, uint64_t hartid)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_TIME);
        kassert((hartid & 0xffull) == hartid);
        cap->word0 = (cap->word0 & ~0xff000000000000ull) | hartid << 48;
}

static inline uint64_t cap_time_get_begin(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_TIME);
        return (cap.word0 >> 32) & 0xffffull;
}

static inline void cap_time_set_begin(cap_t* cap, uint64_t begin)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_TIME);
        kassert((begin & 0xffffull) == begin);
        cap->word0 = (cap->word0 & ~0xffff00000000ull) | begin << 32;
}

static inline uint64_t cap_time_get_end(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_TIME);
        return (cap.word0 >> 16) & 0xffffull;
}

static inline void cap_time_set_end(cap_t* cap, uint64_t end)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_TIME);
        kassert((end & 0xffffull) == end);
        cap->word0 = (cap->word0 & ~0xffff0000ull) | end << 16;
}

static inline uint64_t cap_time_get_free(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_TIME);
        return cap.word0 & 0xffffull;
}

static inline void cap_time_set_free(cap_t* cap, uint64_t free)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_TIME);
        kassert((free & 0xffffull) == free);
        cap->word0 = (cap->word0 & ~0xffffull) | free;
}

static inline uint64_t cap_time_get_depth(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_TIME);
        return cap.word1 & 0xffull;
}

static inline void cap_time_set_depth(cap_t* cap, uint64_t depth)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_TIME);
        kassert((depth & 0xffull) == depth);
        cap->word1 = (cap->word1 & ~0xffull) | depth;
}

static inline cap_t cap_mk_channels(uint64_t begin, uint64_t end, uint64_t free)
{
        kassert((begin & 0xffffull) == begin);
        kassert((end & 0xffffull) == end);
        kassert((free & 0xffffull) == free);
        kassert(begin == free);
        kassert(begin < end);
        kassert(end <= N_CHANNELS);
        cap_t c;
        c.word0 = (uint64_t)CAP_TYPE_CHANNELS << 56;
        c.word1 = 0;
        c.word0 |= free;
        c.word0 |= end << 16;
        c.word0 |= begin << 32;
        return c;
}

static inline uint64_t cap_channels_get_begin(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_CHANNELS);
        return (cap.word0 >> 32) & 0xffffull;
}

static inline void cap_channels_set_begin(cap_t* cap, uint64_t begin)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_CHANNELS);
        kassert((begin & 0xffffull) == begin);
        cap->word0 = (cap->word0 & ~0xffff00000000ull) | begin << 32;
}

static inline uint64_t cap_channels_get_end(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_CHANNELS);
        return (cap.word0 >> 16) & 0xffffull;
}

static inline void cap_channels_set_end(cap_t* cap, uint64_t end)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_CHANNELS);
        kassert((end & 0xffffull) == end);
        cap->word0 = (cap->word0 & ~0xffff0000ull) | end << 16;
}

static inline uint64_t cap_channels_get_free(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_CHANNELS);
        return cap.word0 & 0xffffull;
}

static inline void cap_channels_set_free(cap_t* cap, uint64_t free)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_CHANNELS);
        kassert((free & 0xffffull) == free);
        cap->word0 = (cap->word0 & ~0xffffull) | free;
}

static inline cap_t cap_mk_receiver(uint64_t channel)
{
        kassert((channel & 0xffffull) == channel);
        kassert(channel < N_CHANNELS);
        cap_t c;
        c.word0 = (uint64_t)CAP_TYPE_RECEIVER << 56;
        c.word1 = 0;
        c.word0 |= channel;
        return c;
}

static inline uint64_t cap_receiver_get_channel(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_RECEIVER);
        return cap.word0 & 0xffffull;
}

static inline void cap_receiver_set_channel(cap_t* cap, uint64_t channel)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_RECEIVER);
        kassert((channel & 0xffffull) == channel);
        cap->word0 = (cap->word0 & ~0xffffull) | channel;
}

static inline cap_t cap_mk_sender(uint64_t channel, uint64_t yield)
{
        kassert((channel & 0xffffull) == channel);
        kassert((yield & 0xffull) == yield);
        kassert(channel < N_CHANNELS);
        cap_t c;
        c.word0 = (uint64_t)CAP_TYPE_SENDER << 56;
        c.word1 = 0;
        c.word0 |= yield;
        c.word0 |= channel << 8;
        return c;
}

static inline uint64_t cap_sender_get_channel(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_SENDER);
        return (cap.word0 >> 8) & 0xffffull;
}

static inline void cap_sender_set_channel(cap_t* cap, uint64_t channel)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_SENDER);
        kassert((channel & 0xffffull) == channel);
        cap->word0 = (cap->word0 & ~0xffff00ull) | channel << 8;
}

static inline uint64_t cap_sender_get_yield(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_SENDER);
        return cap.word0 & 0xffull;
}

static inline void cap_sender_set_yield(cap_t* cap, uint64_t yield)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_SENDER);
        kassert((yield & 0xffull) == yield);
        cap->word0 = (cap->word0 & ~0xffull) | yield;
}

static inline cap_t cap_mk_supervisor(uint64_t begin, uint64_t end, uint64_t free)
{
        kassert((begin & 0xffull) == begin);
        kassert((end & 0xffull) == end);
        kassert((free & 0xffull) == free);
        kassert(begin == free);
        kassert(begin < end);
        kassert(end <= N_PROC);
        cap_t c;
        c.word0 = (uint64_t)CAP_TYPE_SUPERVISOR << 56;
        c.word1 = 0;
        c.word0 |= free;
        c.word0 |= end << 8;
        c.word0 |= begin << 16;
        return c;
}

static inline uint64_t cap_supervisor_get_begin(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_SUPERVISOR);
        return (cap.word0 >> 16) & 0xffull;
}

static inline void cap_supervisor_set_begin(cap_t* cap, uint64_t begin)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_SUPERVISOR);
        kassert((begin & 0xffull) == begin);
        cap->word0 = (cap->word0 & ~0xff0000ull) | begin << 16;
}

static inline uint64_t cap_supervisor_get_end(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_SUPERVISOR);
        return (cap.word0 >> 8) & 0xffull;
}

static inline void cap_supervisor_set_end(cap_t* cap, uint64_t end)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_SUPERVISOR);
        kassert((end & 0xffull) == end);
        cap->word0 = (cap->word0 & ~0xff00ull) | end << 8;
}

static inline uint64_t cap_supervisor_get_free(cap_t cap)
{
        kassert((cap.word0 >> 56) == CAP_TYPE_SUPERVISOR);
        return cap.word0 & 0xffull;
}

static inline void cap_supervisor_set_free(cap_t* cap, uint64_t free)
{
        kassert((cap->word0 >> 56) == CAP_TYPE_SUPERVISOR);
        kassert((free & 0xffull) == free);
        cap->word0 = (cap->word0 & ~0xffull) | free;
}

static inline int cap_is_child(cap_t p, cap_t c)
{
        int b = 1;
        cap_type_t parent_type = cap_get_type(p);
        cap_type_t child_type = cap_get_type(c);
        if (parent_type == CAP_TYPE_MEMORY && child_type == CAP_TYPE_MEMORY) {
                b &= cap_memory_get_begin(p) <= cap_memory_get_begin(c);
                b &= cap_memory_get_end(c) <= cap_memory_get_end(p);
                b &= (cap_memory_get_rwx(c) & cap_memory_get_rwx(p)) == cap_memory_get_rwx(c);
                return b;
        }
        if (parent_type == CAP_TYPE_MEMORY && child_type == CAP_TYPE_PMP) {
                b &= cap_memory_get_begin(p) <= pmp_napot_begin(cap_pmp_get_addr(c));
                b &= pmp_napot_end(cap_pmp_get_addr(c)) <= cap_memory_get_end(p);
                b &= (cap_pmp_get_rwx(c) & cap_memory_get_rwx(p)) == cap_pmp_get_rwx(c);
                return b;
        }
        if (parent_type == CAP_TYPE_TIME && child_type == CAP_TYPE_TIME) {
                b &= cap_time_get_begin(p) <= cap_time_get_begin(c);
                b &= cap_time_get_end(c) <= cap_time_get_end(p);
                b &= cap_time_get_hartid(p) == cap_time_get_hartid(c);
                b &= cap_time_get_depth(p) < cap_time_get_depth(c);
                return b;
        }
        if (parent_type == CAP_TYPE_CHANNELS && child_type == CAP_TYPE_CHANNELS) {
                b &= cap_channels_get_begin(p) <= cap_channels_get_begin(c);
                b &= cap_channels_get_end(c) <= cap_channels_get_end(p);
                return b;
        }
        if (parent_type == CAP_TYPE_CHANNELS && child_type == CAP_TYPE_RECEIVER) {
                b &= cap_channels_get_begin(p) <= cap_receiver_get_channel(c);
                b &= cap_receiver_get_channel(c) < cap_channels_get_end(p);
                return b;
        }
        if (parent_type == CAP_TYPE_CHANNELS && child_type == CAP_TYPE_SENDER) {
                b &= cap_channels_get_begin(p) <= cap_sender_get_channel(c);
                b &= cap_sender_get_channel(c) < cap_channels_get_end(p);
                return b;
        }
        if (parent_type == CAP_TYPE_RECEIVER && child_type == CAP_TYPE_SENDER) {
                b &= cap_receiver_get_channel(p) == cap_sender_get_channel(c);
                return b;
        }
        if (parent_type == CAP_TYPE_SUPERVISOR && child_type == CAP_TYPE_SUPERVISOR) {
                b &= cap_supervisor_get_begin(p) <= cap_supervisor_get_begin(c);
                b &= cap_supervisor_get_end(c) <= cap_supervisor_get_end(p);
                return b;
        }
        return 0;
}

static inline int cap_can_derive(cap_t p, cap_t c)
{
        int b = 1;
        cap_type_t parent_type = cap_get_type(p);
        cap_type_t child_type = cap_get_type(c);
        if (parent_type == CAP_TYPE_MEMORY && child_type == CAP_TYPE_MEMORY) {
                b &= cap_memory_get_pmp(p) == 0;
                b &= cap_memory_get_pmp(c) == 0;
                b &= cap_memory_get_free(p) <= cap_memory_get_begin(c);
                b &= cap_memory_get_end(c) <= cap_memory_get_end(p);
                b &= (cap_memory_get_rwx(c) & cap_memory_get_rwx(p)) == cap_memory_get_rwx(c);
                b &= cap_memory_get_free(c) == cap_memory_get_begin(c);
                b &= cap_memory_get_begin(c) < cap_memory_get_end(c);
                return b;
        }
        if (parent_type == CAP_TYPE_MEMORY && child_type == CAP_TYPE_PMP) {
                b &= cap_memory_get_free(p) <= pmp_napot_begin(cap_pmp_get_addr(c));
                b &= pmp_napot_end(cap_pmp_get_addr(c)) <= cap_memory_get_end(p);
                b &= (cap_pmp_get_rwx(c) & cap_memory_get_rwx(p)) == cap_pmp_get_rwx(c);
                return b;
        }
        if (parent_type == CAP_TYPE_TIME && child_type == CAP_TYPE_TIME) {
                b &= cap_time_get_free(p) <= cap_time_get_begin(c);
                b &= cap_time_get_end(c) <= cap_time_get_end(p);
                b &= cap_time_get_hartid(p) == cap_time_get_hartid(c);
                b &= cap_time_get_depth(p) < cap_time_get_depth(c);
                b &= cap_time_get_free(c) == cap_time_get_begin(c);
                b &= cap_time_get_begin(c) < cap_time_get_end(c);
                return b;
        }
        if (parent_type == CAP_TYPE_CHANNELS && child_type == CAP_TYPE_CHANNELS) {
                b &= cap_channels_get_free(p) <= cap_channels_get_begin(c);
                b &= cap_channels_get_end(c) <= cap_channels_get_end(p);
                b &= cap_channels_get_free(c) == cap_channels_get_begin(c);
                b &= cap_channels_get_begin(c) < cap_channels_get_end(c);
                return b;
        }
        if (parent_type == CAP_TYPE_CHANNELS && child_type == CAP_TYPE_RECEIVER) {
                b &= cap_channels_get_free(p) <= cap_receiver_get_channel(c);
                b &= cap_receiver_get_channel(c) < cap_channels_get_end(p);
                return b;
        }
        if (parent_type == CAP_TYPE_RECEIVER && child_type == CAP_TYPE_SENDER) {
                b &= cap_receiver_get_channel(p) == cap_sender_get_channel(c);
                b &= cap_sender_get_yield(c) == 1 || cap_sender_get_yield(c) == 0;
                return b;
        }
        if (parent_type == CAP_TYPE_SUPERVISOR && child_type == CAP_TYPE_SUPERVISOR) {
                b &= cap_supervisor_get_free(p) <= cap_supervisor_get_begin(c);
                b &= cap_supervisor_get_end(c) <= cap_supervisor_get_end(p);
                b &= cap_supervisor_get_free(c) == cap_supervisor_get_begin(c);
                b &= cap_supervisor_get_begin(c) < cap_supervisor_get_end(c);
                return b;
        }
        return 0;
}
