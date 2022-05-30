#pragma once
// See LICENSE file for copyright and license details.

#include <stdint.h>

#include "cap.h"
#include "proc.h"

/* Get process ID */
void SyscallHandler(uint64_t a0, uint64_t a1, uint64_t a2, uint64_t a3,
                    uint64_t a4, uint64_t a5, uint64_t a6, uint64_t a7);

static inline Cap *proc_get_cap(Proc *p, uint64_t idx) {
        return &p->cap_table[idx % N_CAPS];
}

static inline Cap *curr_get_cap(uint64_t idx) {
        return proc_get_cap(current, idx);
}

