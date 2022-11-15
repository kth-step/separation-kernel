#pragma once
#include "kassert.h"
#include <stdint.h>

#define NULL_CAP ((cap_t){0,0})

typedef enum cap_type cap_type_t;
typedef struct cap cap_t;

enum cap_type {
CAP_TYPE_EMPTY, CAP_TYPE_MEMORY, CAP_TYPE_PMP, CAP_TYPE_TIME, CAP_TYPE_CHANNELS, CAP_TYPE_RECEIVER, CAP_TYPE_SENDER, CAP_TYPE_SERVER, CAP_TYPE_CLIENT, CAP_TYPE_SUPERVISOR, NUM_OF_CAP_TYPES
};

struct cap {
unsigned long long word0, word1;
};


static inline uint64_t pmp_napot_begin(uint64_t addr) {
    return addr & (addr + 1);
}

static inline uint64_t pmp_napot_end(uint64_t addr) {
    return addr | (addr + 1);
}

static inline cap_type_t cap_get_type(cap_t cap) {
return 0xffull & cap.word0;
}

static inline int cap_is_type(cap_t cap, cap_type_t t) {
    return cap_get_type(cap) == t;
}

static inline cap_t cap_mk_memory(uint64_t begin, uint64_t end, uint64_t rwx, uint64_t free, uint64_t pmp) {
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
c.word0 = (uint64_t)CAP_TYPE_MEMORY;
c.word1 = 0;
c.word0 |= pmp << 8;
c.word0 |= rwx << 16;
c.word0 |= begin << 24;
c.word1 |= free;
c.word1 |= end << 32;
return c;
}
static inline uint64_t cap_memory_get_begin(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_MEMORY));
return (cap.word0 >> 24) & 0xffffffffull;
}
static inline cap_t cap_memory_set_begin(cap_t cap, uint64_t begin) {
kassert(cap_is_type(cap, CAP_TYPE_MEMORY));
kassert((begin & 0xffffffffull) == begin);
cap.word0 = (cap.word0 & ~0xffffffff000000ull) | begin << 24;
return cap;
}
static inline uint64_t cap_memory_get_end(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_MEMORY));
return (cap.word1 >> 32) & 0xffffffffull;
}
static inline cap_t cap_memory_set_end(cap_t cap, uint64_t end) {
kassert(cap_is_type(cap, CAP_TYPE_MEMORY));
kassert((end & 0xffffffffull) == end);
cap.word1 = (cap.word1 & ~0xffffffff00000000ull) | end << 32;
return cap;
}
static inline uint64_t cap_memory_get_rwx(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_MEMORY));
return (cap.word0 >> 16) & 0xffull;
}
static inline cap_t cap_memory_set_rwx(cap_t cap, uint64_t rwx) {
kassert(cap_is_type(cap, CAP_TYPE_MEMORY));
kassert((rwx & 0xffull) == rwx);
cap.word0 = (cap.word0 & ~0xff0000ull) | rwx << 16;
return cap;
}
static inline uint64_t cap_memory_get_free(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_MEMORY));
return cap.word1 & 0xffffffffull;
}
static inline cap_t cap_memory_set_free(cap_t cap, uint64_t free) {
kassert(cap_is_type(cap, CAP_TYPE_MEMORY));
kassert((free & 0xffffffffull) == free);
cap.word1 = (cap.word1 & ~0xffffffffull) | free;
return cap;
}
static inline uint64_t cap_memory_get_pmp(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_MEMORY));
return (cap.word0 >> 8) & 0xffull;
}
static inline cap_t cap_memory_set_pmp(cap_t cap, uint64_t pmp) {
kassert(cap_is_type(cap, CAP_TYPE_MEMORY));
kassert((pmp & 0xffull) == pmp);
cap.word0 = (cap.word0 & ~0xff00ull) | pmp << 8;
return cap;
}
static inline cap_t cap_mk_pmp(uint64_t addr, uint64_t rwx) {
kassert((addr & 0xffffffffull) == addr);
kassert((rwx & 0xffull) == rwx);
kassert(rwx == 0x4 || rwx == 0x5 || rwx == 0x6 || rwx == 0x7);
cap_t c;
c.word0 = (uint64_t)CAP_TYPE_PMP;
c.word1 = 0;
c.word0 |= rwx << 8;
c.word0 |= addr << 16;
return c;
}
static inline uint64_t cap_pmp_get_addr(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_PMP));
return (cap.word0 >> 16) & 0xffffffffull;
}
static inline cap_t cap_pmp_set_addr(cap_t cap, uint64_t addr) {
kassert(cap_is_type(cap, CAP_TYPE_PMP));
kassert((addr & 0xffffffffull) == addr);
cap.word0 = (cap.word0 & ~0xffffffff0000ull) | addr << 16;
return cap;
}
static inline uint64_t cap_pmp_get_rwx(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_PMP));
return (cap.word0 >> 8) & 0xffull;
}
static inline cap_t cap_pmp_set_rwx(cap_t cap, uint64_t rwx) {
kassert(cap_is_type(cap, CAP_TYPE_PMP));
kassert((rwx & 0xffull) == rwx);
cap.word0 = (cap.word0 & ~0xff00ull) | rwx << 8;
return cap;
}
static inline cap_t cap_mk_time(uint64_t hartid, uint64_t begin, uint64_t end, uint64_t free) {
kassert((hartid & 0xffull) == hartid);
kassert((begin & 0xffffull) == begin);
kassert((end & 0xffffull) == end);
kassert((free & 0xffffull) == free);
kassert(MIN_HARTID <= hartid);
kassert(hartid <= MAX_HARTID);
kassert(begin == free);
kassert(begin < end);
kassert(end <= N_QUANTUM);
cap_t c;
c.word0 = (uint64_t)CAP_TYPE_TIME;
c.word1 = 0;
c.word0 |= free << 8;
c.word0 |= end << 24;
c.word0 |= begin << 40;
c.word0 |= hartid << 56;
return c;
}
static inline uint64_t cap_time_get_hartid(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_TIME));
return (cap.word0 >> 56) & 0xffull;
}
static inline cap_t cap_time_set_hartid(cap_t cap, uint64_t hartid) {
kassert(cap_is_type(cap, CAP_TYPE_TIME));
kassert((hartid & 0xffull) == hartid);
cap.word0 = (cap.word0 & ~0xff00000000000000ull) | hartid << 56;
return cap;
}
static inline uint64_t cap_time_get_begin(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_TIME));
return (cap.word0 >> 40) & 0xffffull;
}
static inline cap_t cap_time_set_begin(cap_t cap, uint64_t begin) {
kassert(cap_is_type(cap, CAP_TYPE_TIME));
kassert((begin & 0xffffull) == begin);
cap.word0 = (cap.word0 & ~0xffff0000000000ull) | begin << 40;
return cap;
}
static inline uint64_t cap_time_get_end(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_TIME));
return (cap.word0 >> 24) & 0xffffull;
}
static inline cap_t cap_time_set_end(cap_t cap, uint64_t end) {
kassert(cap_is_type(cap, CAP_TYPE_TIME));
kassert((end & 0xffffull) == end);
cap.word0 = (cap.word0 & ~0xffff000000ull) | end << 24;
return cap;
}
static inline uint64_t cap_time_get_free(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_TIME));
return (cap.word0 >> 8) & 0xffffull;
}
static inline cap_t cap_time_set_free(cap_t cap, uint64_t free) {
kassert(cap_is_type(cap, CAP_TYPE_TIME));
kassert((free & 0xffffull) == free);
cap.word0 = (cap.word0 & ~0xffff00ull) | free << 8;
return cap;
}
static inline cap_t cap_mk_channels(uint64_t begin, uint64_t end, uint64_t free) {
kassert((begin & 0xffffull) == begin);
kassert((end & 0xffffull) == end);
kassert((free & 0xffffull) == free);
kassert(begin == free);
kassert(begin < end);
kassert(end <= N_CHANNELS);
cap_t c;
c.word0 = (uint64_t)CAP_TYPE_CHANNELS;
c.word1 = 0;
c.word0 |= free << 8;
c.word0 |= end << 24;
c.word0 |= begin << 40;
return c;
}
static inline uint64_t cap_channels_get_begin(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_CHANNELS));
return (cap.word0 >> 40) & 0xffffull;
}
static inline cap_t cap_channels_set_begin(cap_t cap, uint64_t begin) {
kassert(cap_is_type(cap, CAP_TYPE_CHANNELS));
kassert((begin & 0xffffull) == begin);
cap.word0 = (cap.word0 & ~0xffff0000000000ull) | begin << 40;
return cap;
}
static inline uint64_t cap_channels_get_end(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_CHANNELS));
return (cap.word0 >> 24) & 0xffffull;
}
static inline cap_t cap_channels_set_end(cap_t cap, uint64_t end) {
kassert(cap_is_type(cap, CAP_TYPE_CHANNELS));
kassert((end & 0xffffull) == end);
cap.word0 = (cap.word0 & ~0xffff000000ull) | end << 24;
return cap;
}
static inline uint64_t cap_channels_get_free(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_CHANNELS));
return (cap.word0 >> 8) & 0xffffull;
}
static inline cap_t cap_channels_set_free(cap_t cap, uint64_t free) {
kassert(cap_is_type(cap, CAP_TYPE_CHANNELS));
kassert((free & 0xffffull) == free);
cap.word0 = (cap.word0 & ~0xffff00ull) | free << 8;
return cap;
}
static inline cap_t cap_mk_receiver(uint64_t channel, uint64_t grant) {
kassert((channel & 0xffffull) == channel);
kassert((grant & 0xffull) == grant);
kassert(channel < N_CHANNELS);
cap_t c;
c.word0 = (uint64_t)CAP_TYPE_RECEIVER;
c.word1 = 0;
c.word0 |= grant << 8;
c.word0 |= channel << 16;
return c;
}
static inline uint64_t cap_receiver_get_channel(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_RECEIVER));
return (cap.word0 >> 16) & 0xffffull;
}
static inline cap_t cap_receiver_set_channel(cap_t cap, uint64_t channel) {
kassert(cap_is_type(cap, CAP_TYPE_RECEIVER));
kassert((channel & 0xffffull) == channel);
cap.word0 = (cap.word0 & ~0xffff0000ull) | channel << 16;
return cap;
}
static inline uint64_t cap_receiver_get_grant(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_RECEIVER));
return (cap.word0 >> 8) & 0xffull;
}
static inline cap_t cap_receiver_set_grant(cap_t cap, uint64_t grant) {
kassert(cap_is_type(cap, CAP_TYPE_RECEIVER));
kassert((grant & 0xffull) == grant);
cap.word0 = (cap.word0 & ~0xff00ull) | grant << 8;
return cap;
}
static inline cap_t cap_mk_sender(uint64_t channel, uint64_t grant) {
kassert((channel & 0xffffull) == channel);
kassert((grant & 0xffull) == grant);
kassert(channel < N_CHANNELS);
cap_t c;
c.word0 = (uint64_t)CAP_TYPE_SENDER;
c.word1 = 0;
c.word0 |= grant << 8;
c.word0 |= channel << 16;
return c;
}
static inline uint64_t cap_sender_get_channel(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_SENDER));
return (cap.word0 >> 16) & 0xffffull;
}
static inline cap_t cap_sender_set_channel(cap_t cap, uint64_t channel) {
kassert(cap_is_type(cap, CAP_TYPE_SENDER));
kassert((channel & 0xffffull) == channel);
cap.word0 = (cap.word0 & ~0xffff0000ull) | channel << 16;
return cap;
}
static inline uint64_t cap_sender_get_grant(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_SENDER));
return (cap.word0 >> 8) & 0xffull;
}
static inline cap_t cap_sender_set_grant(cap_t cap, uint64_t grant) {
kassert(cap_is_type(cap, CAP_TYPE_SENDER));
kassert((grant & 0xffull) == grant);
cap.word0 = (cap.word0 & ~0xff00ull) | grant << 8;
return cap;
}
static inline cap_t cap_mk_server(uint64_t channel, uint64_t grant) {
kassert((channel & 0xffffull) == channel);
kassert((grant & 0xffull) == grant);
kassert(channel < N_CHANNELS);
cap_t c;
c.word0 = (uint64_t)CAP_TYPE_SERVER;
c.word1 = 0;
c.word0 |= grant << 8;
c.word0 |= channel << 16;
return c;
}
static inline uint64_t cap_server_get_channel(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_SERVER));
return (cap.word0 >> 16) & 0xffffull;
}
static inline cap_t cap_server_set_channel(cap_t cap, uint64_t channel) {
kassert(cap_is_type(cap, CAP_TYPE_SERVER));
kassert((channel & 0xffffull) == channel);
cap.word0 = (cap.word0 & ~0xffff0000ull) | channel << 16;
return cap;
}
static inline uint64_t cap_server_get_grant(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_SERVER));
return (cap.word0 >> 8) & 0xffull;
}
static inline cap_t cap_server_set_grant(cap_t cap, uint64_t grant) {
kassert(cap_is_type(cap, CAP_TYPE_SERVER));
kassert((grant & 0xffull) == grant);
cap.word0 = (cap.word0 & ~0xff00ull) | grant << 8;
return cap;
}
static inline cap_t cap_mk_client(uint64_t channel, uint64_t grant) {
kassert((channel & 0xffffull) == channel);
kassert((grant & 0xffull) == grant);
kassert(channel < N_CHANNELS);
cap_t c;
c.word0 = (uint64_t)CAP_TYPE_CLIENT;
c.word1 = 0;
c.word0 |= grant << 8;
c.word0 |= channel << 16;
return c;
}
static inline uint64_t cap_client_get_channel(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_CLIENT));
return (cap.word0 >> 16) & 0xffffull;
}
static inline cap_t cap_client_set_channel(cap_t cap, uint64_t channel) {
kassert(cap_is_type(cap, CAP_TYPE_CLIENT));
kassert((channel & 0xffffull) == channel);
cap.word0 = (cap.word0 & ~0xffff0000ull) | channel << 16;
return cap;
}
static inline uint64_t cap_client_get_grant(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_CLIENT));
return (cap.word0 >> 8) & 0xffull;
}
static inline cap_t cap_client_set_grant(cap_t cap, uint64_t grant) {
kassert(cap_is_type(cap, CAP_TYPE_CLIENT));
kassert((grant & 0xffull) == grant);
cap.word0 = (cap.word0 & ~0xff00ull) | grant << 8;
return cap;
}
static inline cap_t cap_mk_supervisor(uint64_t begin, uint64_t end, uint64_t free) {
kassert((begin & 0xffull) == begin);
kassert((end & 0xffull) == end);
kassert((free & 0xffull) == free);
kassert(begin == free);
kassert(begin < end);
kassert(end <= N_PROC);
cap_t c;
c.word0 = (uint64_t)CAP_TYPE_SUPERVISOR;
c.word1 = 0;
c.word0 |= free << 8;
c.word0 |= end << 16;
c.word0 |= begin << 24;
return c;
}
static inline uint64_t cap_supervisor_get_begin(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_SUPERVISOR));
return (cap.word0 >> 24) & 0xffull;
}
static inline cap_t cap_supervisor_set_begin(cap_t cap, uint64_t begin) {
kassert(cap_is_type(cap, CAP_TYPE_SUPERVISOR));
kassert((begin & 0xffull) == begin);
cap.word0 = (cap.word0 & ~0xff000000ull) | begin << 24;
return cap;
}
static inline uint64_t cap_supervisor_get_end(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_SUPERVISOR));
return (cap.word0 >> 16) & 0xffull;
}
static inline cap_t cap_supervisor_set_end(cap_t cap, uint64_t end) {
kassert(cap_is_type(cap, CAP_TYPE_SUPERVISOR));
kassert((end & 0xffull) == end);
cap.word0 = (cap.word0 & ~0xff0000ull) | end << 16;
return cap;
}
static inline uint64_t cap_supervisor_get_free(cap_t cap) {
kassert(cap_is_type(cap, CAP_TYPE_SUPERVISOR));
return (cap.word0 >> 8) & 0xffull;
}
static inline cap_t cap_supervisor_set_free(cap_t cap, uint64_t free) {
kassert(cap_is_type(cap, CAP_TYPE_SUPERVISOR));
kassert((free & 0xffull) == free);
cap.word0 = (cap.word0 & ~0xff00ull) | free << 8;
return cap;
}
static inline int cap_is_revokable(cap_t cap) {
return cap_is_type(cap, CAP_TYPE_MEMORY)&&cap_is_type(cap, CAP_TYPE_TIME)&&cap_is_type(cap, CAP_TYPE_CHANNELS)&&cap_is_type(cap, CAP_TYPE_RECEIVER)&&cap_is_type(cap, CAP_TYPE_SENDER)&&cap_is_type(cap, CAP_TYPE_SERVER)&&cap_is_type(cap, CAP_TYPE_CLIENT)&&cap_is_type(cap, CAP_TYPE_SUPERVISOR);
}
static inline int cap_is_child(cap_t p, cap_t c) {
if (cap_is_type(p,CAP_TYPE_MEMORY) && cap_is_type(c, CAP_TYPE_MEMORY))
return (cap_memory_get_begin(p) <= cap_memory_get_begin(c))&&(cap_memory_get_end(c) <= cap_memory_get_free(p))&&((cap_memory_get_rwx(c) & cap_memory_get_rwx(p)) == cap_memory_get_rwx(c));
if (cap_is_type(p,CAP_TYPE_MEMORY) && cap_is_type(c, CAP_TYPE_PMP))
return (cap_memory_get_free(p) <= pmp_napot_begin(cap_pmp_get_addr(c)))&&(pmp_napot_end(cap_pmp_get_addr(c)) <= cap_memory_get_end(p))&&(cap_memory_get_pmp(p) == 1)&&((cap_pmp_get_rwx(c) & cap_memory_get_rwx(p)) == cap_pmp_get_rwx(c));
if (cap_is_type(p,CAP_TYPE_TIME) && cap_is_type(c, CAP_TYPE_TIME))
return (cap_time_get_begin(p) <= cap_time_get_begin(c))&&(cap_time_get_end(c) <= cap_time_get_free(p))&&(cap_time_get_hartid(p) == cap_time_get_hartid(c));
if (cap_is_type(p,CAP_TYPE_CHANNELS) && cap_is_type(c, CAP_TYPE_CHANNELS))
return (cap_channels_get_begin(p) <= cap_channels_get_begin(c))&&(cap_channels_get_end(c) <= cap_channels_get_free(p));
if (cap_is_type(p,CAP_TYPE_CHANNELS) && cap_is_type(c, CAP_TYPE_RECEIVER))
return (cap_channels_get_begin(p) <= cap_receiver_get_channel(c))&&(cap_receiver_get_channel(c) < cap_channels_get_free(p));
if (cap_is_type(p,CAP_TYPE_CHANNELS) && cap_is_type(c, CAP_TYPE_SENDER))
return (cap_channels_get_begin(p) <= cap_sender_get_channel(c))&&(cap_sender_get_channel(c) < cap_channels_get_free(p));
if (cap_is_type(p,CAP_TYPE_CHANNELS) && cap_is_type(c, CAP_TYPE_SERVER))
return (cap_channels_get_begin(p) <= cap_server_get_channel(c))&&(cap_server_get_channel(c) < cap_channels_get_free(p));
if (cap_is_type(p,CAP_TYPE_CHANNELS) && cap_is_type(c, CAP_TYPE_CLIENT))
return (cap_channels_get_begin(p) <= cap_client_get_channel(c))&&(cap_client_get_channel(c) < cap_channels_get_free(p));
if (cap_is_type(p,CAP_TYPE_RECEIVER) && cap_is_type(c, CAP_TYPE_SENDER))
return (cap_receiver_get_channel(p) == cap_sender_get_channel(c));
if (cap_is_type(p,CAP_TYPE_SERVER) && cap_is_type(c, CAP_TYPE_CLIENT))
return (cap_server_get_channel(p) == cap_client_get_channel(c));
if (cap_is_type(p,CAP_TYPE_SUPERVISOR) && cap_is_type(c, CAP_TYPE_SUPERVISOR))
return (cap_supervisor_get_begin(p) <= cap_supervisor_get_begin(c))&&(cap_supervisor_get_end(c) <= cap_supervisor_get_free(p));
return 0;
}
static inline int cap_can_derive(cap_t p, cap_t c) {
if (cap_is_type(p,CAP_TYPE_MEMORY) && cap_is_type(c, CAP_TYPE_MEMORY))
return (cap_memory_get_pmp(p) == 0)&&(cap_memory_get_pmp(c) == 0)&&(cap_memory_get_free(p) == cap_memory_get_begin(c))&&(cap_memory_get_end(c) <= cap_memory_get_end(p))&&((cap_memory_get_rwx(c) & cap_memory_get_rwx(p)) == cap_memory_get_rwx(c))&&(cap_memory_get_free(c) == cap_memory_get_begin(c))&&(cap_memory_get_begin(c) < cap_memory_get_end(c));
if (cap_is_type(p,CAP_TYPE_MEMORY) && cap_is_type(c, CAP_TYPE_PMP))
return (cap_memory_get_free(p) <= pmp_napot_begin(cap_pmp_get_addr(c)))&&(pmp_napot_end(cap_pmp_get_addr(c)) <= cap_memory_get_end(p))&&((cap_pmp_get_rwx(c) & cap_memory_get_rwx(p)) == cap_pmp_get_rwx(c));
if (cap_is_type(p,CAP_TYPE_TIME) && cap_is_type(c, CAP_TYPE_TIME))
return (cap_time_get_free(p) == cap_time_get_begin(c))&&(cap_time_get_end(c) <= cap_time_get_end(p))&&(cap_time_get_hartid(p) == cap_time_get_hartid(c))&&(cap_time_get_free(c) == cap_time_get_begin(c))&&(cap_time_get_begin(c) < cap_time_get_end(c));
if (cap_is_type(p,CAP_TYPE_CHANNELS) && cap_is_type(c, CAP_TYPE_CHANNELS))
return (cap_channels_get_free(p) == cap_channels_get_begin(c))&&(cap_channels_get_end(c) <= cap_channels_get_end(p))&&(cap_channels_get_free(c) == cap_channels_get_begin(c))&&(cap_channels_get_begin(c) < cap_channels_get_end(c));
if (cap_is_type(p,CAP_TYPE_CHANNELS) && cap_is_type(c, CAP_TYPE_RECEIVER))
return (cap_channels_get_free(p) == cap_receiver_get_channel(c))&&(cap_receiver_get_channel(c) < cap_channels_get_end(p))&&(cap_receiver_get_grant(c) == 0 || cap_receiver_get_grant(c) == 1);
if (cap_is_type(p,CAP_TYPE_RECEIVER) && cap_is_type(c, CAP_TYPE_SENDER))
return (cap_receiver_get_channel(p) == cap_sender_get_channel(c))&&(cap_receiver_get_grant(p) == cap_sender_get_grant(c));
if (cap_is_type(p,CAP_TYPE_CHANNELS) && cap_is_type(c, CAP_TYPE_SERVER))
return (cap_channels_get_free(p) == cap_server_get_channel(c))&&(cap_server_get_channel(c) < cap_channels_get_end(p))&&(cap_server_get_grant(c) == 0 || cap_server_get_grant(c) == 1);
if (cap_is_type(p,CAP_TYPE_SERVER) && cap_is_type(c, CAP_TYPE_CLIENT))
return (cap_server_get_channel(p) == cap_client_get_channel(c))&&(cap_server_get_grant(p) == cap_client_get_grant(c));
if (cap_is_type(p,CAP_TYPE_SUPERVISOR) && cap_is_type(c, CAP_TYPE_SUPERVISOR))
return (cap_supervisor_get_free(p) <= cap_supervisor_get_begin(c))&&(cap_supervisor_get_end(c) <= cap_supervisor_get_end(p))&&(cap_supervisor_get_free(c) == cap_supervisor_get_begin(c))&&(cap_supervisor_get_begin(c) < cap_supervisor_get_end(c));
return 0;
}
