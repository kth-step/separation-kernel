// See LICENSE file for copyright and license details.
#pragma once
#include <stdint.h>

enum ordering { relaxed, acquire, release, sequential };

#define fence(pred, succ) __asm__ volatile("fence " #pred "," #succ)

#define __amoop(op, ptr, value, order)                                        \
        ({                                                                    \
                typeof(*ptr) __ret;                                           \
                switch (order) {                                              \
                        case relaxed:                                         \
                                __asm__ volatile("amo" #op ".d %0,%1,(%2)"    \
                                                 : "=r"(__ret)                \
                                                 : "r"(value), "r"(ptr)       \
                                                 : "memory");                 \
                                break;                                        \
                        case acquire:                                         \
                                __asm__ volatile("amo" #op ".d.aq %0,%1,(%2)" \
                                                 : "=r"(__ret)                \
                                                 : "r"(value), "r"(ptr)       \
                                                 : "memory");                 \
                                break;                                        \
                        case release:                                         \
                                __asm__ volatile("amo" #op ".d.rl %0,%1,(%2)" \
                                                 : "=r"(__ret)                \
                                                 : "r"(value), "r"(ptr)       \
                                                 : "memory");                 \
                                break;                                        \
                        case sequential:                                      \
                                __asm__ volatile("amo" #op                    \
                                                 ".d.aqrl %0,%1,(%2)"         \
                                                 : "=r"(__ret)                \
                                                 : "r"(value), "r"(ptr)       \
                                                 : "memory");                 \
                                break;                                        \
                }                                                             \
                __ret;                                                        \
        })

#define amoswap_explicit(ptr, value, order) __amoop(swap, ptr, value, order)
#define amoadd_explicit(ptr, value, order) __amoop(add, ptr, value, order)
#define amoand_explicit(ptr, value, order) __amoop(and, ptr, value, order)
#define amoor_explicit(ptr, value, order) __amoop(or, ptr, value, order)
#define amoxor_explicit(ptr, value, order) __amoop(xor, ptr, value, order)
#define amomax_explicit(ptr, value, order) __amoop(max, ptr, value, order)
#define amomin_explicit(ptr, value, order) __amoop(min, ptr, value, order)
#define amoswap(ptr, value) amoswap_explicit(ptr, value, sequential)
#define amoadd(ptr, value) amoadd_explicit(ptr, value, sequential)
#define amoand(ptr, value) amoand_explicit(ptr, value, sequential)
#define amoor(ptr, value) amoor_explicit(ptr, value, sequential)
#define amoxor(ptr, value) amoxop_explicit(ptr, value, sequential)
#define amomax(ptr, value) amomax_explicit(ptr, value, sequential)
#define amomin(ptr, value) amomin_explicit(ptr, value, sequential)

#define compare_and_swap_explicit(ptr, expected, desired, order)            \
        ({                                                                  \
                typeof(expected) __oldval;                                  \
                uintptr_t __tmp;                                            \
                switch (order) {                                            \
                        case relaxed:                                       \
                                asm volatile(                               \
                                    "1:lr.d %0,(%2)\n"                      \
                                    "  bne %0,%3,2f\n"                      \
                                    "  sc.d %1,%4,(%2)\n"                   \
                                    "  bnez %1,1b\n"                        \
                                    "2:\n"                                  \
                                    : "=r&"(__oldval), "=r&"(__tmp)         \
                                    : "r"(ptr), "r"(expected), "r"(desired) \
                                    : "memory");                            \
                                break;                                      \
                        case acquire:                                       \
                                asm volatile(                               \
                                    "1:lr.d.aq %0,(%2)\n"                   \
                                    "  bne %0,%3,2f\n"                      \
                                    "  sc.d %1,%4,(%2)\n"                   \
                                    "  bnez %1,1b\n"                        \
                                    "2:\n"                                  \
                                    : "=r&"(__oldval), "=r&"(__tmp)         \
                                    : "r"(ptr), "r"(expected), "r"(desired) \
                                    : "memory");                            \
                                break;                                      \
                        case release:                                       \
                                asm volatile(                               \
                                    "1:lr.d %0,(%2)\n"                      \
                                    "  bne %0,%3,2f\n"                      \
                                    "  sc.d.rl %1,%4,(%2)\n"                \
                                    "  bnez %1,1b\n"                        \
                                    "2:\n"                                  \
                                    : "=r&"(__oldval), "=r&"(__tmp)         \
                                    : "r"(ptr), "r"(expected), "r"(desired) \
                                    : "memory");                            \
                                break;                                      \
                        case sequential:                                    \
                                asm volatile(                               \
                                    "1:lr.d.aq %0,(%2)\n"                   \
                                    "  bne %0,%3,2f\n"                      \
                                    "  sc.d.rl %1,%4,(%2)\n"                \
                                    "  bnez %1,1b\n"                        \
                                    "2:\n"                                  \
                                    : "=r&"(__oldval), "=r&"(__tmp)         \
                                    : "r"(ptr), "r"(expected), "r"(desired) \
                                    : "memory");                            \
                                break;                                      \
                }                                                           \
                __oldval == expected;                                       \
        })
#define compare_and_swap(ptr, expected, desired) \
        compare_and_swap_explicit(ptr, expected, desired, sequential)

#define load_explicit(ptr, order)                                      \
        ({                                                             \
                uintptr_t __ret;                                       \
                switch (order) {                                       \
                        case relaxed:                                  \
                                __ret = *ptr;                          \
                                break;                                 \
                        case acquire:                                  \
                                asm volatile("amoor.d.aq %0,x0,(%1)"   \
                                             : "=r"(__ret)             \
                                             : "r"(ptr));              \
                                break;                                 \
                        case release:                                  \
                                asm volatile("amoor.d.rl %0,x0,(%1)"   \
                                             : "=r"(__ret)             \
                                             : "r"(ptr));              \
                                break;                                 \
                        case sequential:                               \
                                asm volatile("amoor.d.aqrl %0,x0,(%1)" \
                                             : "=r"(__ret)             \
                                             : "r"(ptr));              \
                                break;                                 \
                }                                                      \
                __ret;                                                 \
        })
#define load(ptr) load_explicit(ptr, sequential)

#define store_explicit(ptr, value, order)                                     \
        ({                                                                    \
                switch (order) {                                              \
                        case relaxed:                                         \
                                *ptr = value;                                 \
                                break;                                        \
                        case acquire:                                         \
                                asm volatile(                                 \
                                    "amoswap.d.aq x0,%0,(%1)" ::"r"(value),   \
                                    "r"(ptr));                                \
                                break;                                        \
                        case release:                                         \
                                asm volatile(                                 \
                                    "amoswap.d.rl x0,%0,(%1)" ::"r"(value),   \
                                    "r"(ptr));                                \
                                break;                                        \
                        case sequential:                                      \
                                asm volatile(                                 \
                                    "amoswap.d.aqrl x0,%0,(%1)" ::"r"(value), \
                                    "r"(ptr));                                \
                                break;                                        \
                }                                                             \
        })
#define store(ptr) store_explicit(ptr, value, sequential)

/* Macros for marked pointers */
/* Use the most significant bit as mark */
static const unsigned long mark_bit = 1;
#define is_marked(ptr) ((uintptr_t)(ptr)&mark_bit)
#define unmark(ptr) ((void*)((uintptr_t)(ptr) & ~mark_bit))
#define mark(ptr) ((void*)((uintptr_t)(ptr) | mark_bit))
