#pragma once
#include <stdint.h>

typedef struct cap Cap;
typedef enum cap_type CapType;

struct cap {
        unsigned long long word0, word1;
};

enum cap_type {
        CAP_INVALID,
        CAP_MEMORY,
        CAP_PMP,
        CAP_TIME,
        CAP_CHANNELS,
        CAP_ENDPOINT,
        CAP_SUPERVISOR
};

static inline uint64_t pmp_napot_begin(uint64_t addr)
{
        return addr & (addr + 1);
}

static inline uint64_t pmp_napot_end(uint64_t addr)
{
        return addr | (addr + 1);
}

static inline CapType cap_get_type(const Cap cap)
{
        return (cap.word0 >> 56) & 0xff;
}

static inline Cap cap_mk_memory(uint64_t begin, uint64_t end, uint64_t rwx, uint64_t free, uint64_t pmp)
{
        Cap c;
        c.word0 = (uint64_t)CAP_MEMORY << 56;
        c.word1 = 0;
        c.word0 |= pmp;
        c.word0 |= rwx << 8;
        c.word0 |= begin << 16;
        c.word1 |= free;
        c.word1 |= end << 32;
        return c;
}

static inline uint64_t cap_memory_get_begin(const Cap cap)
{
        return (cap.word0 >> 16) & 0xffffffffull;
}

static inline void cap_memory_set_begin(Cap* cap, uint64_t begin)
{
        cap->word0 = (cap->word0 & ~0xffffffff0000ull) | begin << 16;
}

static inline uint64_t cap_memory_get_end(const Cap cap)
{
        return (cap.word1 >> 32) & 0xffffffffull;
}

static inline void cap_memory_set_end(Cap* cap, uint64_t end)
{
        cap->word1 = (cap->word1 & ~0xffffffff00000000ull) | end << 32;
}

static inline uint64_t cap_memory_get_rwx(const Cap cap)
{
        return (cap.word0 >> 8) & 0xffull;
}

static inline void cap_memory_set_rwx(Cap* cap, uint64_t rwx)
{
        cap->word0 = (cap->word0 & ~0xff00ull) | rwx << 8;
}

static inline uint64_t cap_memory_get_free(const Cap cap)
{
        return cap.word1 & 0xffffffffull;
}

static inline void cap_memory_set_free(Cap* cap, uint64_t free)
{
        cap->word1 = (cap->word1 & ~0xffffffffull) | free;
}

static inline uint64_t cap_memory_get_pmp(const Cap cap)
{
        return cap.word0 & 0xffull;
}

static inline void cap_memory_set_pmp(Cap* cap, uint64_t pmp)
{
        cap->word0 = (cap->word0 & ~0xffull) | pmp;
}

static inline Cap cap_mk_pmp(uint64_t addr, uint64_t rwx)
{
        Cap c;
        c.word0 = (uint64_t)CAP_PMP << 56;
        c.word1 = 0;
        c.word0 |= rwx;
        c.word0 |= addr << 8;
        return c;
}

static inline uint64_t cap_pmp_get_addr(const Cap cap)
{
        return (cap.word0 >> 8) & 0xffffffffull;
}

static inline void cap_pmp_set_addr(Cap* cap, uint64_t addr)
{
        cap->word0 = (cap->word0 & ~0xffffffff00ull) | addr << 8;
}

static inline uint64_t cap_pmp_get_rwx(const Cap cap)
{
        return cap.word0 & 0xffull;
}

static inline void cap_pmp_set_rwx(Cap* cap, uint64_t rwx)
{
        cap->word0 = (cap->word0 & ~0xffull) | rwx;
}

static inline Cap cap_mk_time(uint64_t hartid, uint64_t pid, uint64_t begin, uint64_t end, uint64_t free,
    uint64_t depth)
{
        Cap c;
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

static inline uint64_t cap_time_get_hartid(const Cap cap)
{
        return (cap.word0 >> 48) & 0xffull;
}

static inline void cap_time_set_hartid(Cap* cap, uint64_t hartid)
{
        cap->word0 = (cap->word0 & ~0xff000000000000ull) | hartid << 48;
}

static inline uint64_t cap_time_get_pid(const Cap cap)
{
        return (cap.word0 >> 40) & 0xffull;
}

static inline void cap_time_set_pid(Cap* cap, uint64_t pid)
{
        cap->word0 = (cap->word0 & ~0xff0000000000ull) | pid << 40;
}

static inline uint64_t cap_time_get_begin(const Cap cap)
{
        return (cap.word0 >> 24) & 0xffffull;
}

static inline void cap_time_set_begin(Cap* cap, uint64_t begin)
{
        cap->word0 = (cap->word0 & ~0xffff000000ull) | begin << 24;
}

static inline uint64_t cap_time_get_end(const Cap cap)
{
        return (cap.word0 >> 8) & 0xffffull;
}

static inline void cap_time_set_end(Cap* cap, uint64_t end)
{
        cap->word0 = (cap->word0 & ~0xffff00ull) | end << 8;
}

static inline uint64_t cap_time_get_free(const Cap cap)
{
        return cap.word1 & 0xffffull;
}

static inline void cap_time_set_free(Cap* cap, uint64_t free)
{
        cap->word1 = (cap->word1 & ~0xffffull) | free;
}

static inline uint64_t cap_time_get_depth(const Cap cap)
{
        return cap.word0 & 0xffull;
}

static inline void cap_time_set_depth(Cap* cap, uint64_t depth)
{
        cap->word0 = (cap->word0 & ~0xffull) | depth;
}

static inline Cap cap_mk_channels(uint64_t begin, uint64_t end, uint64_t free, uint64_t ep)
{
        Cap c;
        c.word0 = (uint64_t)CAP_CHANNELS << 56;
        c.word1 = 0;
        c.word0 |= ep;
        c.word0 |= free << 8;
        c.word0 |= end << 24;
        c.word0 |= begin << 40;
        return c;
}

static inline uint64_t cap_channels_get_begin(const Cap cap)
{
        return (cap.word0 >> 40) & 0xffffull;
}

static inline void cap_channels_set_begin(Cap* cap, uint64_t begin)
{
        cap->word0 = (cap->word0 & ~0xffff0000000000ull) | begin << 40;
}

static inline uint64_t cap_channels_get_end(const Cap cap)
{
        return (cap.word0 >> 24) & 0xffffull;
}

static inline void cap_channels_set_end(Cap* cap, uint64_t end)
{
        cap->word0 = (cap->word0 & ~0xffff000000ull) | end << 24;
}

static inline uint64_t cap_channels_get_free(const Cap cap)
{
        return (cap.word0 >> 8) & 0xffffull;
}

static inline void cap_channels_set_free(Cap* cap, uint64_t free)
{
        cap->word0 = (cap->word0 & ~0xffff00ull) | free << 8;
}

static inline uint64_t cap_channels_get_ep(const Cap cap)
{
        return cap.word0 & 0xffull;
}

static inline void cap_channels_set_ep(Cap* cap, uint64_t ep)
{
        cap->word0 = (cap->word0 & ~0xffull) | ep;
}

static inline Cap cap_mk_endpoint(uint64_t channel, uint64_t mode)
{
        Cap c;
        c.word0 = (uint64_t)CAP_ENDPOINT << 56;
        c.word1 = 0;
        c.word0 |= mode;
        c.word0 |= channel << 8;
        return c;
}

static inline uint64_t cap_endpoint_get_channel(const Cap cap)
{
        return (cap.word0 >> 8) & 0xffffull;
}

static inline void cap_endpoint_set_channel(Cap* cap, uint64_t channel)
{
        cap->word0 = (cap->word0 & ~0xffff00ull) | channel << 8;
}

static inline uint64_t cap_endpoint_get_mode(const Cap cap)
{
        return cap.word0 & 0xffull;
}

static inline void cap_endpoint_set_mode(Cap* cap, uint64_t mode)
{
        cap->word0 = (cap->word0 & ~0xffull) | mode;
}

static inline Cap cap_mk_supervisor(uint64_t begin, uint64_t end, uint64_t free)
{
        Cap c;
        c.word0 = (uint64_t)CAP_SUPERVISOR << 56;
        c.word1 = 0;
        c.word0 |= free;
        c.word0 |= end << 8;
        c.word0 |= begin << 16;
        return c;
}

static inline uint64_t cap_supervisor_get_begin(const Cap cap)
{
        return (cap.word0 >> 16) & 0xffull;
}

static inline void cap_supervisor_set_begin(Cap* cap, uint64_t begin)
{
        cap->word0 = (cap->word0 & ~0xff0000ull) | begin << 16;
}

static inline uint64_t cap_supervisor_get_end(const Cap cap)
{
        return (cap.word0 >> 8) & 0xffull;
}

static inline void cap_supervisor_set_end(Cap* cap, uint64_t end)
{
        cap->word0 = (cap->word0 & ~0xff00ull) | end << 8;
}

static inline uint64_t cap_supervisor_get_free(const Cap cap)
{
        return cap.word0 & 0xffull;
}

static inline void cap_supervisor_set_free(Cap* cap, uint64_t free)
{
        cap->word0 = (cap->word0 & ~0xffull) | free;
}

static inline bool cap_is_child(const Cap p, const Cap c)
{
        bool b = true;
        CapType parent_type = cap_get_type(p);
        CapType child_type = cap_get_type(c);
        if (parent_type == CAP_MEMORY && child_type == CAP_MEMORY) {
                b &= cap_memory_get_begin(p) <= cap_memory_get_begin(c);
                b &= cap_memory_get_end(c) <= cap_memory_get_end(p);
                b &= (cap_memory_get_rwx(c) & cap_memory_get_rwx(p)) == cap_memory_get_rwx(c);
                return b;
        }
        if (parent_type == CAP_MEMORY && child_type == CAP_PMP) {
                b &= cap_memory_get_begin(p) <= pmp_napot_begin(cap_pmp_get_addr(c));
                b &= pmp_napot_end(cap_pmp_get_addr(c)) <= cap_memory_get_end(p);
                b &= (cap_pmp_get_rwx(c) & cap_memory_get_rwx(p)) == cap_pmp_get_rwx(c);
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
        return false;
}

static inline bool cap_can_derive(const Cap p, const Cap c)
{
        bool b = true;
        CapType parent_type = cap_get_type(p);
        CapType child_type = cap_get_type(c);
        if (parent_type == CAP_MEMORY && child_type == CAP_MEMORY) {
                b &= cap_memory_get_pmp(p) == 0;
                b &= cap_memory_get_pmp(c) == 0;
                b &= cap_memory_get_free(p) <= cap_memory_get_begin(c);
                b &= cap_memory_get_end(c) <= cap_memory_get_end(p);
                b &= (cap_memory_get_rwx(c) & cap_memory_get_rwx(p)) == cap_memory_get_rwx(c);
                b &= cap_memory_get_free(c) == cap_memory_get_begin(c);
                b &= cap_memory_get_begin(c) < cap_memory_get_end(c);
                return b;
        }
        if (parent_type == CAP_MEMORY && child_type == CAP_PMP) {
                b &= cap_memory_get_free(p) <= pmp_napot_begin(cap_pmp_get_addr(c));
                b &= pmp_napot_end(cap_pmp_get_addr(c)) <= cap_memory_get_end(p);
                b &= (cap_pmp_get_rwx(c) & cap_memory_get_rwx(p)) == cap_pmp_get_rwx(c);
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
        return false;
}
