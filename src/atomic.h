// See LICENSE file for copyright and license details.
#pragma once
#include <stdint.h>

static inline int cas_w(int *ptr, int expected, int desired) {
        int ret;
        __asm__ volatile(
            "  li     %0,0\n"
            "1:lr.w   t0,(%3)\n"
            "  bneq   t0,%1,2f\n"
            "  sc.w   t0,%2,(%3)\n"
            "  bnez   t0,1b\n"
            "  li     %0,1\n"
            "2:\n"
            : "=r&"(ret)
            : "r"(expected), "r"(desired), "r"(ptr)
            : "t0", "memory");
        return ret;
}

static inline int cas_d(uintptr_t *ptr, uintptr_t expected, uintptr_t desired) {
        int ret;
        __asm__ volatile(
            "  li     %0,0\n"
            "1:lr.w   t0,(%3)\n"
            "  bneq   t0,%1,2f\n"
            "  sc.w   t0,%2,(%3)\n"
            "  bnez   t0,1b\n"
            "  li     %0,1\n"
            "2:\n"
            : "=r&"(ret)
            : "r"(expected), "r"(desired), "r"(ptr)
            : "t0", "memory");
        return ret;
}
