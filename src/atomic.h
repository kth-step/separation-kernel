// See LICENSE file for copyright and license details.
#pragma once
#include <stdint.h>

#define FENCE(pred, succ) __asm__ volatile("fence " #pred "," #succ)

#define __AMOOP(ptr, value, op, width, aqrl)                              \
        ({                                                                \
                typeof(*ptr) __ret;                                       \
                __asm__ volatile("amo" #op "." #width #aqrl " %0,%1,(%2)" \
                                 : "=r"(__ret)                            \
                                 : "r"(value), "r"(ptr)                   \
                                 : "memory");                             \
                __ret;                                                    \
        })


#define __CAS(ptr, expected, desired, width, aq, rl)        \
        ({                                                  \
                uintptr_t __ret;                            \
                __asm__ volatile(                           \
                    "  li     %0,0\n"                       \
                    "1:lr" #width #aq                       \
                    "   t0,(%3)\n"                          \
                    "  bneq   t0,%1,2f\n"                   \
                    "  sc" #width #rl                       \
                    " t0,%2,(%3)\n"                         \
                    "  bnez   t0,1b\n"                      \
                    "  li     %0,1\n"                       \
                    "2:\n"                                  \
                    : "=r&"(__ret)                          \
                    : "r"(expected), "r"(desired), "r"(ptr) \
                    : "t0", "memory");                      \
                __ret;                                      \
        })

#define AMOOR_D_AQRL(ptr, value) __AMOOP(ptr, value, or, d, .aqrl)
#define AMOAND_D_AQRL(ptr, value) __AMOOP(ptr, value, and, d, .aqrl)

#define CAS_D(ptr, expected, desired) __CAS(ptr, expected, desired, .d, , )
#define CAS_D_AQ(ptr, expected, desired) __CAS(ptr, expected, desired, .d, .aq, )
#define CAS_D_RL(ptr, expected, desired) __CAS(ptr, expected, desired, .d, , .rl)
#define CAS_D_AQRL(ptr, expected, desired) __CAS(ptr, expected, desired, .d, .aq, .rl)

#define CAS_W(ptr, expected, desired) __CAS(ptr, expected, desired, .w, , )
#define CAS_W_AQ(ptr, expected, desired) __CAS(ptr, expected, desired, .w, .aq, )
#define CAS_W_RL(ptr, expected, desired) __CAS(ptr, expected, desired, .w, , .rl)
#define CAS_W_AQRL(ptr, expected, desired) __CAS(ptr, expected, desired, .w, .aq, .rl)

/* Macros for marked pointers */
/* Use the most significant bit as mark */
#define MARK_BIT (1UL << (__riscv_xlen - 1))
#define IS_MARKED(mptr) (mptr >= ((typeof(mptr))MARK_BIT))
#define UNMARK(mptr) ((typeof(mptr))((uintptr_t)mptr & ~MARK_BIT))
#define MARK(mptr) ((typeof(mptr))((uintptr_t)mptr | MARK_BIT))

#if __riscv_xlen == 64
#define CAS(ptr, expected, desired) CAS_D(ptr, expected, desired)
#define CAS_AQ(ptr, expected, desired) CAS_D_AQ(ptr, expected, desired)
#define CAS_RL(ptr, expected, desired) CAS_D_RL(ptr, expected, desired)
#define CAS_AQRL(ptr, expected, desired) CAS_D_AQRL(ptr, expected, desired)
#define AMOOR_AQRL(ptr, value) AMOOR_D_AQRL(ptr, value)
#define AMOAND_AQRL(ptr, value) AMOAND_D_AQRL(ptr, value)
#else
#define CAS(ptr, expected, desired) CAS_W(ptr, expected, desired)
#define CAS_AQ(ptr, expected, desired) CAS_W_AQ(ptr, expected, desired)
#define CAS_RL(ptr, expected, desired) CAS_W_RL(ptr, expected, desired)
#define CAS_AQRL(ptr, expected, desired) CAS_W_AQRL(ptr, expected, desired)
#endif
