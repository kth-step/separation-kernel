// See LICENSE file for copyright and license details.
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "config.h"

typedef struct cap Cap;

struct cap {
        Cap *prev;
        Cap *next;
        uint64_t data[2];
};

extern Cap cap_tables[N_PROC][N_CAPS];
extern volatile int ep_receiver[N_CHANNELS];


_Static_assert(sizeof(Cap) == 32, "Capability node size error");

bool CapRevoke(Cap *cap);
bool CapDelete(Cap *cap);
bool CapMove(Cap *dest, Cap *src);
bool CapAppend(Cap *node, Cap *prev);

bool CapInterprocessMove(Cap *dest, Cap *src, int pid_dest, int pid_src);

Cap* CapInitSentinel(void);

static inline int8_t cap_get_recv(uint16_t epid);
static inline bool cap_cas_recv(uint16_t epid, int old_pid, int new_pid);

int8_t cap_get_recv(uint16_t epid) {
        return ep_receiver[epid];
}

bool cap_cas_recv(uint16_t epid, int old_pid, int new_pid) {
        return __sync_bool_compare_and_swap(&ep_receiver[epid], old_pid, new_pid);
}
