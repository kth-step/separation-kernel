#pragma once
#include "cap.h"
#include "kassert.h"
#include "pmp.h"

typedef enum cap_type CapType;

enum cap_type {
    CAP_INVALID, CAP_MEMORY, CAP_PMP, CAP_PMP_HIDDEN, CAP_TIME, CAP_CHANNELS, CAP_ENDPOINT, CAP_SUPERVISOR
};

static inline CapType cap_get_type(const Cap cap) {
    return (cap.word0 >> 56) & 0xff;
}

const static inline Cap cap_mk_memory(uint64_t begin, uint64_t end, uint64_t rwx, uint64_t free, uint64_t pmp) {
	kassert((begin & 0xffffffffull) == begin);
	kassert((end & 0xffffffffull) == end);
	kassert((rwx & 0xffull) == rwx);
	kassert((free & 0xffffffffull) == free);
	kassert((pmp & 0xffull) == pmp);
	kassert(begin <= free);
	kassert(free <= end);
	kassert(pmp < 2);
	kassert(rwx == 0x4 || rwx == 0x5 || rwx == 0x6 || rwx == 0x7);
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

const static inline uint64_t cap_memory_get_begin(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_MEMORY);
	return (cap.word0 >> 16) & 0xffffffffull;
}

const static inline Cap cap_memory_set_begin(const Cap cap, uint64_t begin) {
	kassert((cap.word0 >> 56) == CAP_MEMORY);
	kassert((begin & 0xffffffffull) == begin);
	Cap c;
	c.word0 = (cap.word0 & ~0xffffffff0000ull) | begin << 16;
	c.word1 = cap.word1;
	return c;
}

const static inline uint64_t cap_memory_get_end(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_MEMORY);
	return (cap.word1 >> 32) & 0xffffffffull;
}

const static inline Cap cap_memory_set_end(const Cap cap, uint64_t end) {
	kassert((cap.word0 >> 56) == CAP_MEMORY);
	kassert((end & 0xffffffffull) == end);
	Cap c;
	c.word0 = cap.word0;
	c.word1 = (cap.word1 & ~0xffffffff00000000ull) | end << 32;
	return c;
}

const static inline uint64_t cap_memory_get_rwx(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_MEMORY);
	return (cap.word0 >> 8) & 0xffull;
}

const static inline Cap cap_memory_set_rwx(const Cap cap, uint64_t rwx) {
	kassert((cap.word0 >> 56) == CAP_MEMORY);
	kassert((rwx & 0xffull) == rwx);
	Cap c;
	c.word0 = (cap.word0 & ~0xff00ull) | rwx << 8;
	c.word1 = cap.word1;
	return c;
}

const static inline uint64_t cap_memory_get_free(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_MEMORY);
	return cap.word1 & 0xffffffffull;
}

const static inline Cap cap_memory_set_free(const Cap cap, uint64_t free) {
	kassert((cap.word0 >> 56) == CAP_MEMORY);
	kassert((free & 0xffffffffull) == free);
	Cap c;
	c.word0 = cap.word0;
	c.word1 = (cap.word1 & ~0xffffffffull) | free;
	return c;
}

const static inline uint64_t cap_memory_get_pmp(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_MEMORY);
	return cap.word0 & 0xffull;
}

const static inline Cap cap_memory_set_pmp(const Cap cap, uint64_t pmp) {
	kassert((cap.word0 >> 56) == CAP_MEMORY);
	kassert((pmp & 0xffull) == pmp);
	Cap c;
	c.word0 = (cap.word0 & ~0xffull) | pmp;
	c.word1 = cap.word1;
	return c;
}

const static inline Cap cap_mk_pmp(uint64_t addr, uint64_t rwx) {
	kassert((addr & 0xffffffffull) == addr);
	kassert((rwx & 0xffull) == rwx);
	kassert(rwx == 0x4 || rwx == 0x5 || rwx == 0x6 || rwx == 0x7);
	Cap c;
	c.word0 = (uint64_t)CAP_PMP << 56;
	c.word1 = 0;
	c.word0 |= rwx;
	c.word0 |= addr << 8;
	return c;
}

const static inline uint64_t cap_pmp_get_addr(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_PMP);
	return (cap.word0 >> 8) & 0xffffffffull;
}

const static inline Cap cap_pmp_set_addr(const Cap cap, uint64_t addr) {
	kassert((cap.word0 >> 56) == CAP_PMP);
	kassert((addr & 0xffffffffull) == addr);
	Cap c;
	c.word0 = (cap.word0 & ~0xffffffff00ull) | addr << 8;
	c.word1 = cap.word1;
	return c;
}

const static inline uint64_t cap_pmp_get_rwx(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_PMP);
	return cap.word0 & 0xffull;
}

const static inline Cap cap_pmp_set_rwx(const Cap cap, uint64_t rwx) {
	kassert((cap.word0 >> 56) == CAP_PMP);
	kassert((rwx & 0xffull) == rwx);
	Cap c;
	c.word0 = (cap.word0 & ~0xffull) | rwx;
	c.word1 = cap.word1;
	return c;
}

const static inline Cap cap_mk_pmp_hidden(uint64_t addr, uint64_t rwx) {
	kassert((addr & 0xffffffffffffffull) == addr);
	kassert((rwx & 0xffull) == rwx);
	kassert(rwx == 0x4 || rwx == 0x5 || rwx == 0x6 || rwx == 0x7);
	Cap c;
	c.word0 = (uint64_t)CAP_PMP_HIDDEN << 56;
	c.word1 = 0;
	c.word1 |= rwx;
	c.word1 |= addr << 8;
	return c;
}

const static inline uint64_t cap_pmp_hidden_get_addr(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_PMP_HIDDEN);
	return (cap.word1 >> 8) & 0xffffffffffffffull;
}

const static inline Cap cap_pmp_hidden_set_addr(const Cap cap, uint64_t addr) {
	kassert((cap.word0 >> 56) == CAP_PMP_HIDDEN);
	kassert((addr & 0xffffffffffffffull) == addr);
	Cap c;
	c.word0 = cap.word0;
	c.word1 = (cap.word1 & ~0xffffffffffffff00ull) | addr << 8;
	return c;
}

const static inline uint64_t cap_pmp_hidden_get_rwx(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_PMP_HIDDEN);
	return cap.word1 & 0xffull;
}

const static inline Cap cap_pmp_hidden_set_rwx(const Cap cap, uint64_t rwx) {
	kassert((cap.word0 >> 56) == CAP_PMP_HIDDEN);
	kassert((rwx & 0xffull) == rwx);
	Cap c;
	c.word0 = cap.word0;
	c.word1 = (cap.word1 & ~0xffull) | rwx;
	return c;
}

const static inline Cap cap_mk_time(uint64_t core, uint64_t pid, uint64_t begin, uint64_t end, uint64_t free, uint64_t depth) {
	kassert((core & 0xffull) == core);
	kassert((pid & 0xffull) == pid);
	kassert((begin & 0xffffull) == begin);
	kassert((end & 0xffffull) == end);
	kassert((free & 0xffffull) == free);
	kassert((depth & 0xffull) == depth);
	kassert(begin <= free);
	kassert(free <= end);
	Cap c;
	c.word0 = (uint64_t)CAP_TIME << 56;
	c.word1 = 0;
	c.word0 |= depth;
	c.word0 |= end << 8;
	c.word0 |= begin << 24;
	c.word0 |= pid << 40;
	c.word0 |= core << 48;
	c.word1 |= free;
	return c;
}

const static inline uint64_t cap_time_get_core(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_TIME);
	return (cap.word0 >> 48) & 0xffull;
}

const static inline Cap cap_time_set_core(const Cap cap, uint64_t core) {
	kassert((cap.word0 >> 56) == CAP_TIME);
	kassert((core & 0xffull) == core);
	Cap c;
	c.word0 = (cap.word0 & ~0xff000000000000ull) | core << 48;
	c.word1 = cap.word1;
	return c;
}

const static inline uint64_t cap_time_get_pid(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_TIME);
	return (cap.word0 >> 40) & 0xffull;
}

const static inline Cap cap_time_set_pid(const Cap cap, uint64_t pid) {
	kassert((cap.word0 >> 56) == CAP_TIME);
	kassert((pid & 0xffull) == pid);
	Cap c;
	c.word0 = (cap.word0 & ~0xff0000000000ull) | pid << 40;
	c.word1 = cap.word1;
	return c;
}

const static inline uint64_t cap_time_get_begin(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_TIME);
	return (cap.word0 >> 24) & 0xffffull;
}

const static inline Cap cap_time_set_begin(const Cap cap, uint64_t begin) {
	kassert((cap.word0 >> 56) == CAP_TIME);
	kassert((begin & 0xffffull) == begin);
	Cap c;
	c.word0 = (cap.word0 & ~0xffff000000ull) | begin << 24;
	c.word1 = cap.word1;
	return c;
}

const static inline uint64_t cap_time_get_end(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_TIME);
	return (cap.word0 >> 8) & 0xffffull;
}

const static inline Cap cap_time_set_end(const Cap cap, uint64_t end) {
	kassert((cap.word0 >> 56) == CAP_TIME);
	kassert((end & 0xffffull) == end);
	Cap c;
	c.word0 = (cap.word0 & ~0xffff00ull) | end << 8;
	c.word1 = cap.word1;
	return c;
}

const static inline uint64_t cap_time_get_free(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_TIME);
	return cap.word1 & 0xffffull;
}

const static inline Cap cap_time_set_free(const Cap cap, uint64_t free) {
	kassert((cap.word0 >> 56) == CAP_TIME);
	kassert((free & 0xffffull) == free);
	Cap c;
	c.word0 = cap.word0;
	c.word1 = (cap.word1 & ~0xffffull) | free;
	return c;
}

const static inline uint64_t cap_time_get_depth(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_TIME);
	return cap.word0 & 0xffull;
}

const static inline Cap cap_time_set_depth(const Cap cap, uint64_t depth) {
	kassert((cap.word0 >> 56) == CAP_TIME);
	kassert((depth & 0xffull) == depth);
	Cap c;
	c.word0 = (cap.word0 & ~0xffull) | depth;
	c.word1 = cap.word1;
	return c;
}

const static inline Cap cap_mk_channels(uint64_t begin, uint64_t end, uint64_t free, uint64_t ep) {
	kassert((begin & 0xffffull) == begin);
	kassert((end & 0xffffull) == end);
	kassert((free & 0xffffull) == free);
	kassert((ep & 0xffull) == ep);
	kassert(begin <= free);
	kassert(free <= begin);
	Cap c;
	c.word0 = (uint64_t)CAP_CHANNELS << 56;
	c.word1 = 0;
	c.word0 |= ep;
	c.word0 |= free << 8;
	c.word0 |= end << 24;
	c.word0 |= begin << 40;
	return c;
}

const static inline uint64_t cap_channels_get_begin(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_CHANNELS);
	return (cap.word0 >> 40) & 0xffffull;
}

const static inline Cap cap_channels_set_begin(const Cap cap, uint64_t begin) {
	kassert((cap.word0 >> 56) == CAP_CHANNELS);
	kassert((begin & 0xffffull) == begin);
	Cap c;
	c.word0 = (cap.word0 & ~0xffff0000000000ull) | begin << 40;
	c.word1 = cap.word1;
	return c;
}

const static inline uint64_t cap_channels_get_end(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_CHANNELS);
	return (cap.word0 >> 24) & 0xffffull;
}

const static inline Cap cap_channels_set_end(const Cap cap, uint64_t end) {
	kassert((cap.word0 >> 56) == CAP_CHANNELS);
	kassert((end & 0xffffull) == end);
	Cap c;
	c.word0 = (cap.word0 & ~0xffff000000ull) | end << 24;
	c.word1 = cap.word1;
	return c;
}

const static inline uint64_t cap_channels_get_free(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_CHANNELS);
	return (cap.word0 >> 8) & 0xffffull;
}

const static inline Cap cap_channels_set_free(const Cap cap, uint64_t free) {
	kassert((cap.word0 >> 56) == CAP_CHANNELS);
	kassert((free & 0xffffull) == free);
	Cap c;
	c.word0 = (cap.word0 & ~0xffff00ull) | free << 8;
	c.word1 = cap.word1;
	return c;
}

const static inline uint64_t cap_channels_get_ep(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_CHANNELS);
	return cap.word0 & 0xffull;
}

const static inline Cap cap_channels_set_ep(const Cap cap, uint64_t ep) {
	kassert((cap.word0 >> 56) == CAP_CHANNELS);
	kassert((ep & 0xffull) == ep);
	Cap c;
	c.word0 = (cap.word0 & ~0xffull) | ep;
	c.word1 = cap.word1;
	return c;
}

const static inline Cap cap_mk_endpoint(uint64_t channel, uint64_t mode, uint64_t grant) {
	kassert((channel & 0xffffull) == channel);
	kassert((mode & 0xffull) == mode);
	kassert((grant & 0xffull) == grant);
	kassert(grant < 2);
	Cap c;
	c.word0 = (uint64_t)CAP_ENDPOINT << 56;
	c.word1 = 0;
	c.word0 |= grant;
	c.word0 |= mode << 8;
	c.word0 |= channel << 16;
	return c;
}

const static inline uint64_t cap_endpoint_get_channel(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_ENDPOINT);
	return (cap.word0 >> 16) & 0xffffull;
}

const static inline Cap cap_endpoint_set_channel(const Cap cap, uint64_t channel) {
	kassert((cap.word0 >> 56) == CAP_ENDPOINT);
	kassert((channel & 0xffffull) == channel);
	Cap c;
	c.word0 = (cap.word0 & ~0xffff0000ull) | channel << 16;
	c.word1 = cap.word1;
	return c;
}

const static inline uint64_t cap_endpoint_get_mode(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_ENDPOINT);
	return (cap.word0 >> 8) & 0xffull;
}

const static inline Cap cap_endpoint_set_mode(const Cap cap, uint64_t mode) {
	kassert((cap.word0 >> 56) == CAP_ENDPOINT);
	kassert((mode & 0xffull) == mode);
	Cap c;
	c.word0 = (cap.word0 & ~0xff00ull) | mode << 8;
	c.word1 = cap.word1;
	return c;
}

const static inline uint64_t cap_endpoint_get_grant(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_ENDPOINT);
	return cap.word0 & 0xffull;
}

const static inline Cap cap_endpoint_set_grant(const Cap cap, uint64_t grant) {
	kassert((cap.word0 >> 56) == CAP_ENDPOINT);
	kassert((grant & 0xffull) == grant);
	Cap c;
	c.word0 = (cap.word0 & ~0xffull) | grant;
	c.word1 = cap.word1;
	return c;
}

const static inline Cap cap_mk_supervisor(uint64_t begin, uint64_t end, uint64_t free) {
	kassert((begin & 0xffull) == begin);
	kassert((end & 0xffull) == end);
	kassert((free & 0xffull) == free);
	kassert(begin <= free);
	kassert(free <= end);
	Cap c;
	c.word0 = (uint64_t)CAP_SUPERVISOR << 56;
	c.word1 = 0;
	c.word0 |= free;
	c.word0 |= end << 8;
	c.word0 |= begin << 16;
	return c;
}

const static inline uint64_t cap_supervisor_get_begin(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_SUPERVISOR);
	return (cap.word0 >> 16) & 0xffull;
}

const static inline Cap cap_supervisor_set_begin(const Cap cap, uint64_t begin) {
	kassert((cap.word0 >> 56) == CAP_SUPERVISOR);
	kassert((begin & 0xffull) == begin);
	Cap c;
	c.word0 = (cap.word0 & ~0xff0000ull) | begin << 16;
	c.word1 = cap.word1;
	return c;
}

const static inline uint64_t cap_supervisor_get_end(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_SUPERVISOR);
	return (cap.word0 >> 8) & 0xffull;
}

const static inline Cap cap_supervisor_set_end(const Cap cap, uint64_t end) {
	kassert((cap.word0 >> 56) == CAP_SUPERVISOR);
	kassert((end & 0xffull) == end);
	Cap c;
	c.word0 = (cap.word0 & ~0xff00ull) | end << 8;
	c.word1 = cap.word1;
	return c;
}

const static inline uint64_t cap_supervisor_get_free(const Cap cap) {
	kassert((cap.word0 >> 56) == CAP_SUPERVISOR);
	return cap.word0 & 0xffull;
}

const static inline Cap cap_supervisor_set_free(const Cap cap, uint64_t free) {
	kassert((cap.word0 >> 56) == CAP_SUPERVISOR);
	kassert((free & 0xffull) == free);
	Cap c;
	c.word0 = (cap.word0 & ~0xffull) | free;
	c.word1 = cap.word1;
	return c;
}

static inline bool cap_is_child(const Cap p, const Cap c) {
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
	if (parent_type == CAP_PMP && child_type == CAP_PMP_HIDDEN) {
		return b;
	}
	if (parent_type == CAP_TIME && child_type == CAP_TIME) {
		b &= cap_time_get_begin(p) <= cap_time_get_begin(c);
		b &= cap_time_get_end(c) <= cap_time_get_end(p);
		b &= cap_time_get_core(p) == cap_time_get_core(c);
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

static inline bool cap_can_derive(const Cap p, const Cap c) {
	bool b = true;
	CapType parent_type = cap_get_type(p);
	CapType child_type = cap_get_type(c);
	if (parent_type == CAP_MEMORY && child_type == CAP_MEMORY) {
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
		b &= cap_time_get_core(p) == cap_time_get_core(c);
		b &= cap_time_get_depth(p) < cap_time_get_depth(c);
		b &= cap_time_get_free(c) == cap_time_get_begin(c);
		b &= cap_time_get_begin(c) < cap_time_get_end(c);
		return b;
	}
	if (parent_type == CAP_CHANNELS && child_type == CAP_CHANNELS) {
		b &= cap_channels_get_free(p) <= cap_channels_get_begin(c);
		b &= cap_channels_get_end(c) <= cap_channels_get_end(p);
		b &= cap_channels_get_free(c) == cap_channels_get_begin(c);
		b &= cap_channels_get_begin(c) < cap_channels_get_end(c);
		return b;
	}
	if (parent_type == CAP_CHANNELS && child_type == CAP_ENDPOINT) {
		b &= cap_channels_get_free(p) <= cap_endpoint_get_channel(c);
		b &= cap_endpoint_get_channel(c) < cap_channels_get_end(p);
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

