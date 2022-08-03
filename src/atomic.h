#pragma once
#include <stdbool.h>
#include <stdint.h>

#define synchronize() __sync_synchronize()
#define compare_and_set(ptr, expected, desired) __sync_bool_compare_and_swap(ptr, expected, desired)
#define compare_and_swap(ptr, expected, desired) __sync_val_compare_and_swap(ptr, expected, desired)
#define fetch_and_and(ptr, val) __sync_fetch_and_and(ptr, val)
#define fetch_and_or(ptr, val) __sync_fetch_and_or(ptr, val)
#define fetch_and_add(ptr, val) __sync_fetch_and_add(ptr, val)

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
