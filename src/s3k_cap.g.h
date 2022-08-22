#pragma once
#include "config.h"
#include "pmp.h"

#define NULL_CAP ((cap_t){0,0})

typedef enum cap_type cap_type_t;
typedef struct cap cap_t;

enum cap_type {
CAP_TYPE_EMPTY, CAP_TYPE_MEMORY, CAP_TYPE_PMP, CAP_TYPE_TIME, CAP_TYPE_CHANNELS, CAP_TYPE_RECEIVER, CAP_TYPE_SENDER, CAP_TYPE_SERVER, CAP_TYPE_CLIENT, CAP_TYPE_SUPERVISOR, NUM_OF_CAP_TYPES
};

struct cap {
unsigned long long word0, word1;
};

static inline cap_type_t cap_get_type(cap_t cap) {
return (cap.word0 >> 56) & 0xff;
}

static inline cap_t cap_mk_memory(uint64_t begin, uint64_t end, uint64_t rwx, uint64_t free, uint64_t pmp) {
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
static inline uint64_t cap_memory_get_begin(cap_t cap) {
return (cap.word0 >> 16) & 0xffffffffull;
}
static inline void cap_memory_set_begin(cap_t *cap, uint64_t begin) {
cap->word0 = (cap->word0 & ~0xffffffff0000ull) | begin << 16;
}
static inline uint64_t cap_memory_get_end(cap_t cap) {
return (cap.word1 >> 32) & 0xffffffffull;
}
static inline void cap_memory_set_end(cap_t *cap, uint64_t end) {
cap->word1 = (cap->word1 & ~0xffffffff00000000ull) | end << 32;
}
static inline uint64_t cap_memory_get_rwx(cap_t cap) {
return (cap.word0 >> 8) & 0xffull;
}
static inline void cap_memory_set_rwx(cap_t *cap, uint64_t rwx) {
cap->word0 = (cap->word0 & ~0xff00ull) | rwx << 8;
}
static inline uint64_t cap_memory_get_free(cap_t cap) {
return cap.word1 & 0xffffffffull;
}
static inline void cap_memory_set_free(cap_t *cap, uint64_t free) {
cap->word1 = (cap->word1 & ~0xffffffffull) | free;
}
static inline uint64_t cap_memory_get_pmp(cap_t cap) {
return cap.word0 & 0xffull;
}
static inline void cap_memory_set_pmp(cap_t *cap, uint64_t pmp) {
cap->word0 = (cap->word0 & ~0xffull) | pmp;
}
static inline cap_t cap_mk_pmp(uint64_t addr, uint64_t rwx) {
cap_t c;
c.word0 = (uint64_t)CAP_TYPE_PMP << 56;
c.word1 = 0;
c.word0 |= rwx;
c.word0 |= addr << 8;
return c;
}
static inline uint64_t cap_pmp_get_addr(cap_t cap) {
return (cap.word0 >> 8) & 0xffffffffull;
}
static inline void cap_pmp_set_addr(cap_t *cap, uint64_t addr) {
cap->word0 = (cap->word0 & ~0xffffffff00ull) | addr << 8;
}
static inline uint64_t cap_pmp_get_rwx(cap_t cap) {
return cap.word0 & 0xffull;
}
static inline void cap_pmp_set_rwx(cap_t *cap, uint64_t rwx) {
cap->word0 = (cap->word0 & ~0xffull) | rwx;
}
static inline cap_t cap_mk_time(uint64_t hartid, uint64_t begin, uint64_t end, uint64_t free, uint64_t depth) {
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
static inline uint64_t cap_time_get_hartid(cap_t cap) {
return (cap.word0 >> 48) & 0xffull;
}
static inline void cap_time_set_hartid(cap_t *cap, uint64_t hartid) {
cap->word0 = (cap->word0 & ~0xff000000000000ull) | hartid << 48;
}
static inline uint64_t cap_time_get_begin(cap_t cap) {
return (cap.word0 >> 32) & 0xffffull;
}
static inline void cap_time_set_begin(cap_t *cap, uint64_t begin) {
cap->word0 = (cap->word0 & ~0xffff00000000ull) | begin << 32;
}
static inline uint64_t cap_time_get_end(cap_t cap) {
return (cap.word0 >> 16) & 0xffffull;
}
static inline void cap_time_set_end(cap_t *cap, uint64_t end) {
cap->word0 = (cap->word0 & ~0xffff0000ull) | end << 16;
}
static inline uint64_t cap_time_get_free(cap_t cap) {
return cap.word0 & 0xffffull;
}
static inline void cap_time_set_free(cap_t *cap, uint64_t free) {
cap->word0 = (cap->word0 & ~0xffffull) | free;
}
static inline uint64_t cap_time_get_depth(cap_t cap) {
return cap.word1 & 0xffull;
}
static inline void cap_time_set_depth(cap_t *cap, uint64_t depth) {
cap->word1 = (cap->word1 & ~0xffull) | depth;
}
static inline cap_t cap_mk_channels(uint64_t begin, uint64_t end, uint64_t free) {
cap_t c;
c.word0 = (uint64_t)CAP_TYPE_CHANNELS << 56;
c.word1 = 0;
c.word0 |= free;
c.word0 |= end << 16;
c.word0 |= begin << 32;
return c;
}
static inline uint64_t cap_channels_get_begin(cap_t cap) {
return (cap.word0 >> 32) & 0xffffull;
}
static inline void cap_channels_set_begin(cap_t *cap, uint64_t begin) {
cap->word0 = (cap->word0 & ~0xffff00000000ull) | begin << 32;
}
static inline uint64_t cap_channels_get_end(cap_t cap) {
return (cap.word0 >> 16) & 0xffffull;
}
static inline void cap_channels_set_end(cap_t *cap, uint64_t end) {
cap->word0 = (cap->word0 & ~0xffff0000ull) | end << 16;
}
static inline uint64_t cap_channels_get_free(cap_t cap) {
return cap.word0 & 0xffffull;
}
static inline void cap_channels_set_free(cap_t *cap, uint64_t free) {
cap->word0 = (cap->word0 & ~0xffffull) | free;
}
static inline cap_t cap_mk_receiver(uint64_t channel) {
cap_t c;
c.word0 = (uint64_t)CAP_TYPE_RECEIVER << 56;
c.word1 = 0;
c.word0 |= channel;
return c;
}
static inline uint64_t cap_receiver_get_channel(cap_t cap) {
return cap.word0 & 0xffffull;
}
static inline void cap_receiver_set_channel(cap_t *cap, uint64_t channel) {
cap->word0 = (cap->word0 & ~0xffffull) | channel;
}
static inline cap_t cap_mk_sender(uint64_t channel, uint64_t grant) {
cap_t c;
c.word0 = (uint64_t)CAP_TYPE_SENDER << 56;
c.word1 = 0;
c.word0 |= grant;
c.word0 |= channel << 8;
return c;
}
static inline uint64_t cap_sender_get_channel(cap_t cap) {
return (cap.word0 >> 8) & 0xffffull;
}
static inline void cap_sender_set_channel(cap_t *cap, uint64_t channel) {
cap->word0 = (cap->word0 & ~0xffff00ull) | channel << 8;
}
static inline uint64_t cap_sender_get_grant(cap_t cap) {
return cap.word0 & 0xffull;
}
static inline void cap_sender_set_grant(cap_t *cap, uint64_t grant) {
cap->word0 = (cap->word0 & ~0xffull) | grant;
}
static inline cap_t cap_mk_server(uint64_t channel, uint64_t grant) {
cap_t c;
c.word0 = (uint64_t)CAP_TYPE_SERVER << 56;
c.word1 = 0;
c.word0 |= grant;
c.word0 |= channel << 8;
return c;
}
static inline uint64_t cap_server_get_channel(cap_t cap) {
return (cap.word0 >> 8) & 0xffffull;
}
static inline void cap_server_set_channel(cap_t *cap, uint64_t channel) {
cap->word0 = (cap->word0 & ~0xffff00ull) | channel << 8;
}
static inline uint64_t cap_server_get_grant(cap_t cap) {
return cap.word0 & 0xffull;
}
static inline void cap_server_set_grant(cap_t *cap, uint64_t grant) {
cap->word0 = (cap->word0 & ~0xffull) | grant;
}
static inline cap_t cap_mk_client(uint64_t channel, uint64_t grant) {
cap_t c;
c.word0 = (uint64_t)CAP_TYPE_CLIENT << 56;
c.word1 = 0;
c.word0 |= grant;
c.word0 |= channel << 8;
return c;
}
static inline uint64_t cap_client_get_channel(cap_t cap) {
return (cap.word0 >> 8) & 0xffffull;
}
static inline void cap_client_set_channel(cap_t *cap, uint64_t channel) {
cap->word0 = (cap->word0 & ~0xffff00ull) | channel << 8;
}
static inline uint64_t cap_client_get_grant(cap_t cap) {
return cap.word0 & 0xffull;
}
static inline void cap_client_set_grant(cap_t *cap, uint64_t grant) {
cap->word0 = (cap->word0 & ~0xffull) | grant;
}
static inline cap_t cap_mk_supervisor(uint64_t begin, uint64_t end, uint64_t free) {
cap_t c;
c.word0 = (uint64_t)CAP_TYPE_SUPERVISOR << 56;
c.word1 = 0;
c.word0 |= free;
c.word0 |= end << 8;
c.word0 |= begin << 16;
return c;
}
static inline uint64_t cap_supervisor_get_begin(cap_t cap) {
return (cap.word0 >> 16) & 0xffull;
}
static inline void cap_supervisor_set_begin(cap_t *cap, uint64_t begin) {
cap->word0 = (cap->word0 & ~0xff0000ull) | begin << 16;
}
static inline uint64_t cap_supervisor_get_end(cap_t cap) {
return (cap.word0 >> 8) & 0xffull;
}
static inline void cap_supervisor_set_end(cap_t *cap, uint64_t end) {
cap->word0 = (cap->word0 & ~0xff00ull) | end << 8;
}
static inline uint64_t cap_supervisor_get_free(cap_t cap) {
return cap.word0 & 0xffull;
}
static inline void cap_supervisor_set_free(cap_t *cap, uint64_t free) {
cap->word0 = (cap->word0 & ~0xffull) | free;
}
static inline int cap_is_child_memory(cap_t p, cap_t c) {
cap_type_t parent_type = cap_get_type(p);
cap_type_t child_type = cap_get_type(c);
if (parent_type == CAP_TYPE_MEMORY && child_type == CAP_TYPE_MEMORY)
return (cap_memory_get_begin(p) <= cap_memory_get_begin(c))&&(cap_memory_get_end(c) <= cap_memory_get_end(p))&&((cap_memory_get_rwx(c) & cap_memory_get_rwx(p)) == cap_memory_get_rwx(c));
if (parent_type == CAP_TYPE_MEMORY && child_type == CAP_TYPE_PMP)
return (cap_memory_get_begin(p) <= pmp_napot_begin(cap_pmp_get_addr(c)))&&(pmp_napot_end(cap_pmp_get_addr(c)) <= cap_memory_get_end(p))&&((cap_pmp_get_rwx(c) & cap_memory_get_rwx(p)) == cap_pmp_get_rwx(c));
return 0;
}
static inline int cap_is_child_time(cap_t p, cap_t c) {
cap_type_t parent_type = cap_get_type(p);
cap_type_t child_type = cap_get_type(c);
if (parent_type == CAP_TYPE_TIME && child_type == CAP_TYPE_TIME)
return (cap_time_get_begin(p) <= cap_time_get_begin(c))&&(cap_time_get_end(c) <= cap_time_get_end(p))&&(cap_time_get_hartid(p) == cap_time_get_hartid(c))&&(cap_time_get_depth(p) < cap_time_get_depth(c));
return 0;
}
static inline int cap_is_child_channels(cap_t p, cap_t c) {
cap_type_t parent_type = cap_get_type(p);
cap_type_t child_type = cap_get_type(c);
if (parent_type == CAP_TYPE_CHANNELS && child_type == CAP_TYPE_CHANNELS)
return (cap_channels_get_begin(p) <= cap_channels_get_begin(c))&&(cap_channels_get_end(c) <= cap_channels_get_end(p));
if (parent_type == CAP_TYPE_CHANNELS && child_type == CAP_TYPE_RECEIVER)
return (cap_channels_get_begin(p) <= cap_receiver_get_channel(c))&&(cap_receiver_get_channel(c) < cap_channels_get_end(p));
if (parent_type == CAP_TYPE_CHANNELS && child_type == CAP_TYPE_SENDER)
return (cap_channels_get_begin(p) <= cap_sender_get_channel(c))&&(cap_sender_get_channel(c) < cap_channels_get_end(p));
if (parent_type == CAP_TYPE_CHANNELS && child_type == CAP_TYPE_SERVER)
return (cap_channels_get_begin(p) <= cap_server_get_channel(c))&&(cap_server_get_channel(c) < cap_channels_get_end(p));
if (parent_type == CAP_TYPE_CHANNELS && child_type == CAP_TYPE_CLIENT)
return (cap_channels_get_begin(p) <= cap_client_get_channel(c))&&(cap_client_get_channel(c) < cap_channels_get_end(p));
return 0;
}
static inline int cap_is_child_receiver(cap_t p, cap_t c) {
cap_type_t parent_type = cap_get_type(p);
cap_type_t child_type = cap_get_type(c);
if (parent_type == CAP_TYPE_RECEIVER && child_type == CAP_TYPE_SENDER)
return (cap_receiver_get_channel(p) == cap_sender_get_channel(c));
return 0;
}
static inline int cap_is_child_server(cap_t p, cap_t c) {
cap_type_t parent_type = cap_get_type(p);
cap_type_t child_type = cap_get_type(c);
if (parent_type == CAP_TYPE_SERVER && child_type == CAP_TYPE_CLIENT)
return (cap_server_get_channel(p) == cap_client_get_channel(c));
return 0;
}
static inline int cap_is_child_supervisor(cap_t p, cap_t c) {
cap_type_t parent_type = cap_get_type(p);
cap_type_t child_type = cap_get_type(c);
if (parent_type == CAP_TYPE_SUPERVISOR && child_type == CAP_TYPE_SUPERVISOR)
return (cap_supervisor_get_begin(p) <= cap_supervisor_get_begin(c))&&(cap_supervisor_get_end(c) <= cap_supervisor_get_end(p));
return 0;
}
static inline int cap_can_derive_memory(cap_t p, cap_t c) {
cap_type_t parent_type = cap_get_type(p);
cap_type_t child_type = cap_get_type(c);
if (parent_type == CAP_TYPE_MEMORY && child_type == CAP_TYPE_MEMORY)
return (cap_memory_get_pmp(p) == 0)&&(cap_memory_get_pmp(c) == 0)&&(cap_memory_get_free(p) <= cap_memory_get_begin(c))&&(cap_memory_get_end(c) <= cap_memory_get_end(p))&&((cap_memory_get_rwx(c) & cap_memory_get_rwx(p)) == cap_memory_get_rwx(c))&&(cap_memory_get_free(c) == cap_memory_get_begin(c))&&(cap_memory_get_begin(c) < cap_memory_get_end(c));
if (parent_type == CAP_TYPE_MEMORY && child_type == CAP_TYPE_PMP)
return (cap_memory_get_free(p) <= pmp_napot_begin(cap_pmp_get_addr(c)))&&(pmp_napot_end(cap_pmp_get_addr(c)) <= cap_memory_get_end(p))&&((cap_pmp_get_rwx(c) & cap_memory_get_rwx(p)) == cap_pmp_get_rwx(c));
return 0;
}
static inline int cap_can_derive_time(cap_t p, cap_t c) {
cap_type_t parent_type = cap_get_type(p);
cap_type_t child_type = cap_get_type(c);
if (parent_type == CAP_TYPE_TIME && child_type == CAP_TYPE_TIME)
return (cap_time_get_free(p) <= cap_time_get_begin(c))&&(cap_time_get_end(c) <= cap_time_get_end(p))&&(cap_time_get_hartid(p) == cap_time_get_hartid(c))&&(cap_time_get_depth(p) < cap_time_get_depth(c))&&(cap_time_get_free(c) == cap_time_get_begin(c))&&(cap_time_get_begin(c) < cap_time_get_end(c));
return 0;
}
static inline int cap_can_derive_channels(cap_t p, cap_t c) {
cap_type_t parent_type = cap_get_type(p);
cap_type_t child_type = cap_get_type(c);
if (parent_type == CAP_TYPE_CHANNELS && child_type == CAP_TYPE_CHANNELS)
return (cap_channels_get_free(p) <= cap_channels_get_begin(c))&&(cap_channels_get_end(c) <= cap_channels_get_end(p))&&(cap_channels_get_free(c) == cap_channels_get_begin(c))&&(cap_channels_get_begin(c) < cap_channels_get_end(c));
if (parent_type == CAP_TYPE_CHANNELS && child_type == CAP_TYPE_RECEIVER)
return (cap_channels_get_free(p) <= cap_receiver_get_channel(c))&&(cap_receiver_get_channel(c) < cap_channels_get_end(p));
if (parent_type == CAP_TYPE_CHANNELS && child_type == CAP_TYPE_SERVER)
return (cap_channels_get_free(p) <= cap_server_get_channel(c))&&(cap_server_get_channel(c) < cap_channels_get_end(p));
return 0;
}
static inline int cap_can_derive_receiver(cap_t p, cap_t c) {
cap_type_t parent_type = cap_get_type(p);
cap_type_t child_type = cap_get_type(c);
if (parent_type == CAP_TYPE_RECEIVER && child_type == CAP_TYPE_SENDER)
return (cap_receiver_get_channel(p) == cap_sender_get_channel(c))&&(cap_sender_get_grant(c) == 1 || cap_sender_get_grant(c) == 0);
return 0;
}
static inline int cap_can_derive_server(cap_t p, cap_t c) {
cap_type_t parent_type = cap_get_type(p);
cap_type_t child_type = cap_get_type(c);
if (parent_type == CAP_TYPE_SERVER && child_type == CAP_TYPE_CLIENT)
return (cap_server_get_channel(p) == cap_client_get_channel(c))&&(cap_client_get_grant(c) == 1 || cap_client_get_grant(c) == 0);
return 0;
}
static inline int cap_can_derive_supervisor(cap_t p, cap_t c) {
cap_type_t parent_type = cap_get_type(p);
cap_type_t child_type = cap_get_type(c);
if (parent_type == CAP_TYPE_SUPERVISOR && child_type == CAP_TYPE_SUPERVISOR)
return (cap_supervisor_get_free(p) <= cap_supervisor_get_begin(c))&&(cap_supervisor_get_end(c) <= cap_supervisor_get_end(p))&&(cap_supervisor_get_free(c) == cap_supervisor_get_begin(c))&&(cap_supervisor_get_begin(c) < cap_supervisor_get_end(c));
return 0;
}
