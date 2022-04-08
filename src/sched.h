// See LICENSE file for copyright and license details.
#pragma once

#include <stdint.h>

#include "config.h"
#include "proc.h"
#include "timer.h"

void Sched(void);

extern void AsmSwitchToSched();
static inline void sched_preemption() {
        if (__builtin_expect(read_csr(mip) & 128,0)) {
                AsmSwitchToSched();
        }
}
