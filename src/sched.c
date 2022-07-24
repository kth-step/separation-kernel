// See LICENSE file for copyright and license details.
#include "sched.h"

#include <stddef.h>

#include "cap_node.h"
#include "csr.h"
#include "lock.h"
#include "proc_state.h"

#define SCHED_SLICE_OFFSET(sched_slice, hartid) ((sched_slice) << ((hartid)-MIN_HARTID) * 16)

static uint64_t schedule[N_QUANTUM];
Lock lock = 0;

/* Returns pid and depth on an sched_slice for a hart */
static inline uint64_t sched_entry(uint64_t sched_slice, uint64_t hartid);

/* Returns pid on an sched_slice for a hart */
static inline uint64_t sched_pid(uint64_t sched_slice, uint64_t hartid);

/* Gets the processes that should run on _hartid_ at time _time_. */
static void sched_get_proc(uint64_t hartid, uint64_t time, proc_t** proc, uint64_t* length);

/**************** Definitions ****************/
uint64_t sched_entry(uint64_t s, uint64_t hartid)
{
        kassert(MIN_HARTID <= hartid && hartid <= MAX_HARTID);
        s >>= ((hartid)-MIN_HARTID) * 16;
        return s & 0xFFFF;
}

uint64_t sched_pid(uint64_t s, uint64_t hartid)
{
        return sched_entry(s, hartid) & 0xFF;
}

void sched_get_proc(uint64_t hartid, uint64_t time, proc_t** proc, uint64_t* length)
{
        kassert(MIN_HARTID <= hartid && hartid <= MAX_HARTID);

        /* Set proc to NULL as default */
        *proc = NULL;
        *length = 0;
        /* Calculate the current quantum */
        uint64_t quantum = time % N_QUANTUM;
        /* Get the current quantum schedule */
        __sync_synchronize();
        uint64_t sched_slice = schedule[quantum];
        uint64_t pid = sched_pid(sched_slice, hartid);

        /* Check if slot is invalid/inactive */
        if (pid == 0xFF)
                return;

        /* Check if some other thread preempts */
        for (size_t i = MIN_HARTID; i < hartid; i++) {
                /* If the pid is same, there is hart with higher priortiy */
                uint64_t other_pid = sched_pid(sched_slice, i);
                if (pid == other_pid)
                        return;
        }

        /* Calculate the length of the same time slice */
        uint64_t quantum_end = quantum + 1;
        while (quantum_end < N_QUANTUM && sched_entry(sched_slice, hartid) == sched_entry(schedule[quantum_end], hartid))
                quantum_end++;

        /* Set proc and length */
        *proc = &processes[pid];
        *length = quantum_end - quantum;
        return;
}

void wait_and_set_timeout(uint64_t time, uint64_t length, uint64_t timeout)
{
        uint64_t start_time = time * TICKS;
        uint64_t end_time = start_time + length * TICKS - SCHEDULER_TICKS;
        uint64_t hartid = read_csr(mhartid);
        if (timeout > start_time)
                start_time = timeout;
        write_timeout(hartid, start_time);
        while (!(read_csr(mip) & 128))
                asm volatile("wfi");
        write_timeout(hartid, end_time);
}

void sched_start(void)
{
        uintptr_t hartid = read_csr(mhartid);
        /* Process to run and number of time slices to run for */
        proc_t* proc = NULL;
        uint64_t time, length, timeout = 0, end_time;

        while (1) {
                /* Get the current time */
                time = (read_time() / TICKS) + 1;
                /* Try getting a process at that time slice. */
                sched_get_proc(hartid, time, &proc, &length);
                if (proc == NULL)
                        continue;
                timeout = proc->regs.timeout;
                end_time = (time + length) * TICKS - SCHEDULER_TICKS;
                if (timeout >= end_time)
                        continue;
                if (proc_acquire(proc))
                        break;
        }
        /* Wait for time slice to start and set timeout */
        current = proc;
        wait_and_set_timeout(time, length, timeout);
}

void sched(void)
{
        /* The hart/core id */
        proc_release(current);
        sched_start();
}

bool sched_update(cap_node_t* cn, uint64_t hartid, uint64_t begin, uint64_t end, uint64_t expected_depth, uint64_t desired_pid, uint64_t desired_depth)
{
        kassert(begin < end);
        kassert(end <= N_QUANTUM);
        kassert((expected_depth & 0xFF) == expected_depth);
        kassert((desired_pid & 0xFF) == desired_pid);
        kassert((desired_depth & 0xFF) == desired_depth);

        lock_acquire(&lock);
        if (cap_node_is_deleted(cn)) {
                lock_release(&lock);
                return false;
        }

        /* Expected value */
        uint64_t expected = SCHED_SLICE_OFFSET(expected_depth << 8, hartid);

        /* Desired value */
        uint64_t desired = SCHED_SLICE_OFFSET(desired_depth << 8 | desired_pid, hartid);

        /* Mask so we match on desired entry */
        uint64_t mask_desired = ~SCHED_SLICE_OFFSET(0xFFFF, hartid);
        uint64_t mask_expected = SCHED_SLICE_OFFSET(0xFF00, hartid);

        for (int i = begin; i < end; ++i) {
                uint64_t expected_s = schedule[i];
                if ((expected_s & mask_expected) != expected)
                        continue;
                uint64_t desired_s = (expected_s & mask_desired) | desired;
                schedule[i] = desired_s;
        }
        lock_release(&lock);
        return true;
}
