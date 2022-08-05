// See LICENSE file for copyright and license details.
#pragma once
#include "atomic.h"

typedef struct lock {
        volatile int now_serving;
        volatile int next_ticket;
} lock_t;

#define INIT_LOCK ((lock_t){0, 0})

static inline void lock_acquire(lock_t* lock);
static inline void lock_release(lock_t* lock);

void lock_acquire(lock_t* lock)
{
        int ticket = fetch_and_add(&lock->next_ticket, 1);
        while (ticket != lock->now_serving)
                ;
}

void lock_release(lock_t* lock)
{
        synchronize();
        lock->now_serving++;
}
