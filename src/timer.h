// See LICENSE file for copyright and license details.
#pragma once

#include "config.h"
#include "types.h"
#include "csr.h"

uint64_t *const mtime = (uint64_t*)0x200bff8;
uint64_t *const mtimecmp = (uint64_t*)0x2004000;

inline uint64_t read_time(void) {
        return *mtime;
}

inline void write_time(uint64_t time) {
        *mtime = time;
}

inline uint64_t read_timeout(uintptr_t hartid) {
        return mtimecmp[hartid];
}

inline void write_timeout(uintptr_t hartid, uint64_t timeout) {
        mtimecmp[hartid] = timeout;
}
