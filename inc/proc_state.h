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
static inline bool proc_server_acquire(proc_t* proc, uint64_t channel);
static inline void proc_server_release(proc_t* proc);
static inline bool proc_client_wait(proc_t* proc, uint64_t channel);

void proc_release(proc_t* proc)
{
        if (!(proc->state & S3K_STATE_WAITING)) {
                /* If not waiting, then go to suspend */
                fetch_and_and(&proc->state, S3K_STATE_SUSPENDED);
        }
}

bool proc_acquire(proc_t* proc)
{
        return compare_and_swap(&proc->state, S3K_STATE_READY, S3K_STATE_RUNNING);
}

bool proc_is_suspended(proc_t* proc)
{
        return (proc->state & S3K_STATE_SUSPENDED) != 0;
}

bool proc_supervisor_acquire(proc_t* proc)
{
        return compare_and_swap(&proc->state, S3K_STATE_SUSPENDED, S3K_STATE_SUSPENDED_BUSY);
}

void proc_supervisor_release(proc_t* proc)
{
        kassert(proc->state == S3K_STATE_SUSPENDED_BUSY);
        synchronize();
        proc->state = S3K_STATE_SUSPENDED;
}

bool proc_supervisor_resume(proc_t* proc)
{
        if (proc_supervisor_acquire(proc)) {
                proc->state = S3K_STATE_READY;
                return true;
        }
        return false;
}

bool proc_supervisor_suspend(proc_t* proc)
{
        /* Set the suspended bit */
        uint64_t state = fetch_and_or(&proc->state, S3K_STATE_SUSPENDED);
        if (state & S3K_STATE_SUSPENDED)
                return false;
        if ((state & 0x7) == S3K_STATE_WAITING)
                proc->state = S3K_STATE_SUSPENDED;
        return true;
}

bool proc_receiver_wait(proc_t* proc, uint64_t channel)
{
        uint64_t expected = S3K_STATE_RUNNING;
        uint64_t desired = channel << 48 | S3K_STATE_WAITING;
        return compare_and_swap(&proc->state, expected, desired);
}

bool proc_sender_acquire(proc_t* proc, uint64_t channel)
{
        uint64_t expected = channel << 48 | S3K_STATE_WAITING;
        uint64_t desired = channel << 48 | S3K_STATE_RECEIVING;
        return compare_and_swap(&proc->state, expected, desired);
}

bool proc_client_wait(proc_t* proc, uint64_t channel)
{
        uint64_t expected = S3K_STATE_RUNNING;
        uint64_t desired = channel << 48 | S3K_STATE_WAITING | (1ull << (__riscv_xlen - 1));
        return compare_and_swap(&proc->state, expected, desired);
}

void proc_sender_release(proc_t* proc)
{
        fetch_and_and(&proc->state, S3K_STATE_SUSPENDED);
}

bool proc_server_acquire(proc_t* proc, uint64_t channel)
{
        uint64_t expected = channel << 48 | S3K_STATE_WAITING;
        uint64_t desired = channel << 48 | S3K_STATE_RECEIVING | (1ull << (__riscv_xlen - 1));
        return compare_and_swap(&proc->state, expected, desired);
}

void proc_server_release(proc_t* proc)
{
        fetch_and_and(&proc->state, S3K_STATE_SUSPENDED);
}
