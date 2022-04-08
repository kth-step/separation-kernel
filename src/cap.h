// See LICENSE file for copyright and license details.
#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "atomic.h"
#include "config.h"

typedef struct cap Cap;

struct cap {
        Cap *prev;
        Cap *next;
        uint64_t data[2];
};

extern Cap cap_tables[N_PROC][N_CAPS];

_Static_assert(sizeof(Cap) == 32, "Capability node size error");

void CapRevoke(Cap *cap);
bool CapDelete(Cap *cap);
bool CapMove(Cap *dest, Cap *src);
bool CapAppend(Cap *node, Cap *prev);

uint64_t CapSliceTS(Cap *src, Cap *dest, uint64_t data[2]);
uint64_t CapSplitTS(Cap *src, Cap *dest0, Cap *dest1, uint64_t data0[2], uint64_t data1[2]);

uint64_t CapSliceMS(Cap *src, Cap *dest, uint64_t data[2]);
uint64_t CapSplitMS(Cap *src, Cap *dest0, Cap *dest1, uint64_t data0[2], uint64_t data1[2]);
