// See LICENSE file for copyright and license details.
#pragma once
#include <stdint.h>

static inline void pmp_napot_bounds(uint64_t addr, uint64_t *begin, uint64_t *end) {
        *begin = addr & (addr + 1);
        *end = addr | (addr + 1);
}

static inline void set_pmp(uint64_t *pmpcfg, uint64_t pmpaddr[8], uint64_t config[8]) {
        *pmpcfg = 0;
        for (int i = 0; i < 8; i++) {
                pmpaddr[i] = config[i] >> 8;
                *pmpcfg |= (config[i] & 0xFF) << (i * 8);
        }
}
