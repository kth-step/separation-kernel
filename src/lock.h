#pragma once

typedef volatile int Lock;

static inline void lock_acquire(Lock* lock);
static inline int lock_try_acquire(Lock* lock);
static inline void lock_release(Lock* lock);

void lock_acquire(Lock* lock)
{
        while (!__sync_bool_compare_and_swap(lock, 0, 1))
                ;
}

int lock_try_acquire(Lock* lock)
{
        return __sync_bool_compare_and_swap(lock, 0, 1);
}

static inline void lock_release(Lock* lock)
{
        __sync_synchronize();
        *lock = 0;
}
