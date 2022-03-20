// See LICENSE file for copyright and license details.
#pragma once
#include <stdint.h>

typedef unsigned long Lock;

static inline
uintptr_t try_acquire_lock(Lock *l) {
        register uintptr_t t0;
        __asm__ volatile (
                "1:lr.d.aq %0,(%1)\n"
                "  bnez %0,2f\n"
                "  sc.d %0,%1,(%1)\n"
                "  bnez %0,1b\n"
                "2:\n" :"=r&"(t0):"r"(l):"memory");
        return t0;
}

static inline
void acquire_lock(Lock *l) {
        __asm__ volatile (
                "1:lr.d.aq t0,(%0)\n"
                "  bnez t0,1b\n"
                "  sc.d t0,%0,(%0)\n"
                "  bnez t0,1b\n"
                ::"r"(l):"t0","memory");
}

static inline
void release_lock(Lock *l) {
        __asm__ volatile ("amoswap.d.rl zero,zero,(%0)"::"r"(l):"memory");
}
