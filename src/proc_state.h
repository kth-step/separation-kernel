// See LICENSE file for copyright and license details.
#pragma once
#include "atomic.h"
#include "proc.h"

static inline void proc_release(proc_t* proc);
static inline bool proc_acquire(proc_t* proc);
static inline bool proc_is_suspended(proc_t* proc);
static inline bool proc_supervisor_acquire(proc_t* proc);
static inline void proc_supervisor_release(proc_t* proc);
static inline bool proc_supervisor_resume(proc_t* proc);
static inline bool proc_supervisor_suspend(proc_t* proc);
static inline bool proc_receiver_wait(proc_t* proc, uint64_t channel);
static inline bool proc_sender_acquire(proc_t* proc, uint64_t channel);
static inline void proc_sender_release(proc_t* proc);

void proc_release(proc_t* proc)
{
        if (!(proc->state & PROC_STATE_WAITING)) {
                /* If not waiting, then go to suspend */
                fetch_and_and(&proc->state, PROC_STATE_SUSPENDED);
        }
}

bool proc_acquire(proc_t* proc)
{
        return compare_and_set(&proc->state, PROC_STATE_READY, PROC_STATE_RUNNING);
}

bool proc_is_suspended(proc_t* proc)
{
        return (proc->state & PROC_STATE_SUSPENDED) != 0;
}

bool proc_supervisor_acquire(proc_t* proc)
{
        return compare_and_set(&proc->state, PROC_STATE_SUSPENDED, PROC_STATE_SUSPENDED_BUSY);
}

void proc_supervisor_release(proc_t* proc)
{
        kassert(proc->state == PROC_STATE_SUSPENDED_BUSY);
        synchronize();
        proc->state = PROC_STATE_SUSPENDED;
}

bool proc_supervisor_resume(proc_t* proc)
{
        if (proc_supervisor_acquire(proc)) {
                proc->state = PROC_STATE_READY;
                return S3K_OK;
        }
        return S3K_SUPERVISEE_BUSY;
}

bool proc_supervisor_suspend(proc_t* proc)
{
        /* Set the suspended bit */
        uint64_t state = fetch_and_or(&proc->state, PROC_STATE_SUSPENDED);
        if ((state & 7) == PROC_STATE_WAITING) {
                proc->state = PROC_STATE_SUSPENDED;
        }
        return S3K_OK;
}

bool proc_receiver_wait(proc_t* proc, uint64_t channel)
{
        uint64_t expected = PROC_STATE_RUNNING;
        uint64_t desired = channel << 48 | PROC_STATE_WAITING;
        return compare_and_set(&current->state, expected, desired);
}

bool proc_sender_acquire(proc_t* proc, uint64_t channel)
{
        uint64_t expected = channel << 48 | PROC_STATE_WAITING;
        uint64_t desired = channel << 48 | PROC_STATE_RECEIVING;
        return compare_and_set(&proc->state, expected, desired);
}

void proc_sender_release(proc_t* proc)
{
        fetch_and_and(&proc->state, PROC_STATE_SUSPENDED);
}
