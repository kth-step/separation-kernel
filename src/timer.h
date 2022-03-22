// See LICENSE file for copyright and license details.
#pragma once
#include <stdint.h>

#include "config.h"
#include "csr.h"

extern volatile uint64_t mtime;
extern volatile uint64_t mtimecmp[N_CORES];

inline uint64_t read_time(void) {
        return mtime;
}

inline void write_time(uint64_t time) {
        mtime = time;
}

inline uint64_t read_timeout(uintptr_t hartid) {
        return mtimecmp[hartid];
}

inline void write_timeout(uintptr_t hartid, uint64_t timeout) {
        mtimecmp[hartid] = timeout;
}
