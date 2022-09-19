// See LICENSE file for copyright and license details.
#pragma once
#include <stdint.h>

#include "csr.h"

static inline uint64_t pmp_napot_begin(uint64_t addr)
{
    return addr & (addr + 1);
}

static inline uint64_t pmp_napot_end(uint64_t addr)
{
    return addr | (addr + 1);
}
