// See LICENSE file for copyright and license details.
#pragma once
#include <stdbool.h>
#include <stdint.h>

#define synchronize() __sync_synchronize()
#define fetch_and_and(ptr, val) __sync_fetch_and_and(ptr, val)
#define fetch_and_or(ptr, val) __sync_fetch_and_or(ptr, val)
#define fetch_and_add(ptr, val) __sync_fetch_and_add(ptr, val)

#ifndef BUILTIN_ATOMIC
// the builtin atomic compare_and_swap/set always uses registers a0 and a1 as temporary registers,
// selects the temporary registers dynamically, reducing the amount of stack usage.
#define compare_and_swap(ptr, expected, desired)          \
    ({                                                    \
        register uint64_t tmp0, tmp1;                     \
        if (sizeof(*ptr) == 8) {                          \
            __asm__ volatile(                             \
                "1:lr.d.aq %0,(%2)\n"                     \
                "bne    %0,%3,2f\n"                       \
                "sc.d %1,%4,(%2)\n"                       \
                "bnez   %1,1b\n"                          \
                "2:"                                      \
                : "=&r"(tmp0), "=&r"(tmp1)                \
                : "r"(ptr), "r"(expected), "r"(desired)); \
        } else if (sizeof(*ptr) == 4) {                   \
            __asm__ volatile(                             \
                "1:lr.w.aq %0,(%2)\n"                     \
                "bne    %0,%3,2f\n"                       \
                "sc.w.rl %1,%4,(%2)\n"                    \
                "bnez   %1,1b\n"                          \
                "2:"                                      \
                : "=&r"(tmp0), "=&r"(tmp1)                \
                : "r"(ptr), "r"(expected), "r"(desired)); \
        } else {                                          \
            while (1)                                     \
                ;                                         \
        }                                                 \
        (typeof(*ptr)) tmp0;                              \
    })
#define compare_and_set(ptr, expected, desired) (compare_and_swap(ptr, expected, desired) == expected)
#else
#define compare_and_set(ptr, expected, desired) __sync_bool_compare_and_swap(ptr, expected, desired)
#define compare_and_swap(ptr, expected, desired) __sync_val_compare_and_swap(ptr, expected, desired)
#endif

typedef struct marked_pointer {
    uint64_t data;
} marked_pointer_t;

static inline marked_pointer_t marked_pointer(void* ptr, bool mark)
{
    return (marked_pointer_t){(uint64_t)ptr | mark};
}

static inline void* marked_pointer_get_ptr(marked_pointer_t* mptr)
{
    return (void*)(mptr->data & ~1ull);
}

static inline void* marked_pointer_get(marked_pointer_t* mptr, int* mark)
{
    uint64_t data = mptr->data;
    *mark = (int)(data & 1ull);
    return (void*)(data & ~1ull);
}

static inline bool marked_pointer_compare_and_set(marked_pointer_t* ptr, void* old_ptr, void* new_ptr, int old_mark,
                                                  int new_mark)
{
    uint64_t old_data = (uint64_t)old_ptr | old_mark;
    uint64_t new_data = (uint64_t)new_ptr | new_mark;
    return compare_and_set(&ptr->data, old_data, new_data);
}
