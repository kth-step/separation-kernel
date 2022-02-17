// See LICENSE file for copyright and license details.

#pragma once
#include "types.h"

typedef uintptr_t Lock;

inline void acquire_lock(Lock *lock) {
        register uintptr_t t0,t1;
        asm volatile (
                "li %1,1\n"
                "1:lr.d.aq %0,0(%2)\n" 
                "bnez %0,1b\n"
                "sc.d %0,%1,0(%2)\n"
                "bnez %0,1b\n"
                : "=r"(t0), "=r"(t1)
                : "r"(lock) : "memory");
}

// Returns zero success, non-zero on fail.
inline uintptr_t try_acquire_lock(Lock *lock) {
        register uintptr_t t0,t1;
        asm volatile (
                "li %1,1\n"
                "lr.d.aq %0,0(%2)\n" 
                "bnez %0,1f\n"
                "sc.d %0,%1,0(%2)\n"
                "1:"
                : "=r"(t0), "=r"(t1)
                : "r"(lock) : "memory");
        return t0;
}

inline void release_lock(Lock *lock) {
        asm volatile (
                "fence rw,w\n"
                "sd x0,0(%0)"
                :: "r"(lock) : "memory");
}
