// See LICENSE file for copyright and license details.
#pragma once
#include <stdint.h>

typedef unsigned long Lock;

static inline uintptr_t try_acquire_lock(Lock *l) {
        return !__sync_lock_test_and_set(l, 1);
}

static inline void acquire_lock(Lock *l) {
        while (__sync_lock_test_and_set(l, 1))
                ;
}

static inline void release_lock(Lock *l) {
        __sync_lock_release(l, 0);
}
