// See LICENSE file for copyright and license details.
#include "sched.h"

#include <stddef.h>

#include "cap.h"
#include "csr.h"
#include "lock.h"
#include "stack.h"

#define SCHED_SLICE_OFFSET(sched_slice, hartid) \
        ((sched_slice) << ((hartid)-MIN_HARTID) * 16)

static uint64_t schedule[N_QUANTUM];
static Lock lock = 0;

/* Returns pid and depth on an sched_slice for a hart */
static inline uint64_t sched_entry(uint64_t sched_slice, uint64_t hartid);

/* Returns pid on an sched_slice for a hart */
static inline uint64_t sched_pid(uint64_t sched_slice, uint64_t hartid);

/* Returns pid on an sched_slice for a hart */
static inline uint64_t sched_depth(uint64_t sched_slice, uint64_t hartid);

/* Gets the processes that should run on _hartid_ at time _time_. */
static void sched_get_proc(uint64_t hartid, uint64_t time, Proc **proc,
                           uint64_t *length);

static inline bool sched_update(uint64_t begin, uint64_t end, uint64_t mask,
                                uint64_t expected, uint64_t desired,
                                CapNode *cn);

/**************** Definitions ****************/
uint64_t sched_entry(uint64_t s, uint64_t hartid) {
        kassert(MIN_HARTID <= hartid && hartid <= MAX_HARTID);
        s >>= ((hartid)-MIN_HARTID) * 16;
        return s & 0xFFFF;
}

uint64_t sched_pid(uint64_t s, uint64_t hartid) {
        return sched_entry(s, hartid) & 0xFF;
}

uint64_t sched_depth(uint64_t s, uint64_t hartid) {
        return (sched_entry(s, hartid) >> 8) & 0xFF;
}

void sched_get_proc(uint64_t hartid, uint64_t time, Proc **proc,
                    uint64_t *length) {
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
        if (pid & 0x80)
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
        while (quantum_end < N_QUANTUM &&
               sched_entry(sched_slice, hartid) ==
                   sched_entry(schedule[quantum_end], hartid))
                quantum_end++;

        /* Set proc and length */
        *proc = &processes[pid];
        *length = quantum_end - quantum;
        return;
}

void wait_and_set_timeout(uint64_t time, uint64_t length) {
        uint64_t start_time = time * TICKS;
        uint64_t end_time = start_time + length * TICKS - SCHEDULER_TICKS;
        uint64_t hartid = read_csr(mhartid);
        write_timeout(hartid, start_time);
        while (!(read_csr(mip) & 128))
                asm volatile("wfi");
        write_timeout(hartid, end_time);
}

void Sched(void) {
        /* The hart/core id */
        uintptr_t hartid = read_csr(mhartid);

        /* Process to run and number of time slices to run for */
        Proc *proc = NULL;
        uint64_t time, length;

        do {
                /* Get the current time */
                time = (read_time() / TICKS) + 1;
                /* Try getting a process at that time slice. */
                sched_get_proc(hartid, time, &proc, &length);
        } while (!proc);
        /* Wait for time slice to start and set timeout */
        current = proc;
        wait_and_set_timeout(time, length);
}

bool sched_update(uint64_t begin, uint64_t end, uint64_t mask,
                  uint64_t expected, uint64_t desired, CapNode *cn) {
        /* Disable preemption */
        clear_csr(mstatus, 8);
        /* Try acquire lock */
        while (!lock_try_acquire(&lock)) {
                /* If failed acquire lock, enable preemption temporary */
                set_csr(mstatus, 8);
                clear_csr(mstatus, 8);
        }
        /* Check that the capability still exists */
        if (cn->prev == NULL) {
                /* If capability deleted, release lock, enable preemption and
                 * return */
                lock_release(&lock);
                set_csr(mstatus, 8);
                return false;
        }
        /* Update the schedule */
        for (int i = begin; i < end; i++) {
                uint64_t sched_slice = schedule[i];
                if ((sched_slice & mask) != expected)
                        continue;
                uint64_t new_sched_slice = (sched_slice & ~mask) | desired;
                schedule[i] = new_sched_slice;
        }
        /* Release lock and enable preemption */
        lock_release(&lock);
        set_csr(mstatus, 8);
        return true;
}

bool SchedRevoke(const Cap cap, CapNode *cn) {
        uint64_t hartid = cap_time_get_hartid(cap);
        uint64_t begin = cap_time_get_begin(cap);
        uint64_t end = cap_time_get_end(cap);

        uint64_t depth = cap_time_get_depth(cap);
        uint64_t pid = cap_time_get_pid(cap);

        uint64_t mask = SCHED_SLICE_OFFSET(0xFFFF, hartid);
        uint64_t desired = SCHED_SLICE_OFFSET(depth << 8 | pid, hartid);

        lock_acquire(&lock);

        if (cn->prev == NULL) {
                lock_release(&lock);
                return false;
        }

        for (int i = begin; i < end; i++) {
                /* For the revoke to be correct, we assume that
                 * it is impossible to delete the time slice (cn)
                 * and create a new time slice with lower depth between the
                 * first if statement and the compare and swap. */
                uint64_t sched_slice = schedule[i];
                uint64_t new_sched_slice = (sched_slice & ~mask) | desired;
                schedule[i] = new_sched_slice;
        }
        lock_release(&lock);
        return true;
}

bool SchedUpdate(const Cap cap, const Cap new_cap, CapNode *cn) {
        kassert(cap_get_type(cap) == CAP_TIME);
        kassert(cap_get_type(new_cap) == CAP_TIME);
        uint64_t hartid = cap_time_get_hartid(new_cap);
        uint64_t begin = cap_time_get_begin(new_cap);
        uint64_t end = cap_time_get_end(new_cap);

        uint64_t old_depth = cap_time_get_depth(cap);
        uint64_t old_pid = cap_time_get_pid(cap);

        uint64_t new_depth = cap_time_get_depth(new_cap);
        uint64_t new_pid = cap_time_get_pid(new_cap);

        /* Expected value */
        uint64_t expected =
            SCHED_SLICE_OFFSET(old_depth << 8 | old_pid, hartid);
        /* Desired value */
        uint64_t desired = SCHED_SLICE_OFFSET(new_depth << 8 | new_pid, hartid);
        /* Mask so we match on desired entry */
        uint64_t mask = SCHED_SLICE_OFFSET(0xFFFF, hartid);
        return sched_update(begin, end, mask, expected, desired, cn);
}

bool SchedDelete(const Cap cap, CapNode *cn) {
        kassert(cap_get_type(cap) == CAP_TIME);
        uint64_t depth = cap_time_get_depth(cap);
        uint64_t pid = cap_time_get_pid(cap);
        uint64_t begin = cap_time_get_begin(cap);
        uint64_t end = cap_time_get_end(cap);
        uint64_t hartid = cap_time_get_hartid(cap);

        /* Expected value */
        uint64_t expected = SCHED_SLICE_OFFSET(depth << 8 | pid, hartid);
        /* Desired value, set pid to invalid/inactive */
        uint64_t desired = SCHED_SLICE_OFFSET(0x0080, hartid);
        /* Mask so we match on desired entry */
        uint64_t mask = SCHED_SLICE_OFFSET(0xFFFF, hartid);
        return sched_update(begin, end, mask, expected, desired, cn);
}
