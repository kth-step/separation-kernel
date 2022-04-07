// See LICENSE file for copyright and license details.
#pragma once
#include <stdint.h>

#include "atomic.h"

typedef unsigned long Lock;

static inline uintptr_t try_acquire_lock(Lock *l) {
        return amoor_explicit(l, 1, acquire);
}

static inline void acquire_lock(Lock *l) {
        while (!amoor_explicit(l, 1, acquire))
                ;
}

static inline void release_lock(Lock *l) {
        store_explicit(l, 0, release);
}
