// See LICENSE file for copyright and license details.
#pragma once
#include <stdint.h>

typedef unsigned long Lock;

static inline uintptr_t try_acquire_lock(Lock *l) {
        register uintptr_t res;
        __asm__ volatile(
            "  li           %0,0\n"
            "1:lr.d.aq      t0,(%1)\n"
            "  bnez         t0,2f\n"
            "  sc.d         t0,%1,(%1)\n"
            "  bnez         t0,1b\n"
            "  li           %0,1\n"
            "2:\n"
            : "=r&"(res)
            : "r"(l)
            : "t0", "memory");
        return res;
}

static inline void acquire_lock(Lock *l) {
        __asm__ volatile(
            "1:lr.d.aq      t0,(%0)\n"
            "  bnez         t0,1b\n"
            "  sc.d         t0,%0,(%0)\n"
            "  bnez         t0,1b\n"
            : /* No output */
            : "r"(l)
            : "t0", "memory");
}

static inline void release_lock(Lock *l) {
        __asm__ volatile("amoswap.d.rl   x0,x0,(%0)"
                         : /* No output */
                         : "r"(l)
                         : "memory");
}
