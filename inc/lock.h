// See LICENSE file for copyright and license details.
#pragma once

typedef volatile int Lock;

inline void acquire_lock(Lock *lock) {
        asm volatile (
                "li t1,1\n"
                "1:lr.w.aq t2,0(%0)\n" 
                "bnez t2,1b\n"
                "sc.w t2,t1,0(%0)\n"
                "bnez t2,1b\n"
                :: "r"(lock) : "t2","t1", "memory");
}

// Returns zero success, non-zero on fail.
inline int try_acquire_lock(Lock *lock) {
        int t0;
        asm volatile (
                "li t0,1\n"
                "1:lr.w.aq %0,0(%1)\n" 
                "bnez %0,2f\n"
                "sc.w %0,t0,0(%1)\n"
                "bnez %0,1b\n"
                "2:"
                : "=r"(t0) : "r"(lock) : "t0", "memory");
        return t0;
}

inline void release_lock(Lock *lock) {
        asm volatile (
                "fence rw,w\n"
                "sw x0,0(%0)"
                :: "r"(lock) : "memory");
}
