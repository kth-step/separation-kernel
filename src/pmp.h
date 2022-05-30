#pragma once
#include <stdint.h>

static inline void pmp_napot_bounds(uint64_t addr, uint64_t *begin, uint64_t *end) {
        addr = addr << 2 | 0x3;
        *begin = addr & (addr + 1);
        *end = addr | (addr + 1);
}
