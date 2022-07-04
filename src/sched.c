// See LICENSE file for copyright and license details.
#include "sched.h"

#include <stddef.h>

#include "cap.h"
#include "csr.h"
#include "stack.h"

/** The schedule.
 * Each 64-bit word, sched_slice, describes the state of four cores:
 *   [(pid,depth), (pid,depth), (pid,depth), (pid,depth)]
 *   (lower bits are pid, upper are depth)
 * The pid describes what process runs on a core, depth (with quantum) uniquely
 * identifies which capability is used to scheduler the process.
 *
 * Note: We should probably replace uint64_t with appropriate structs.
 */
static uint64_t schedule[N_QUANTUM];

/* Returns pid and depth on an sched_slice for a hart */
static inline uint64_t sched_entry(uint64_t sched_slice, uint64_t hartid);

/* Returns pid on an sched_slice for a hart */
static inline uint64_t sched_pid(uint64_t sched_slice, uint64_t hartid);

/* Returns pid on an sched_slice for a hart */
static inline uint64_t sched_depth(uint64_t sched_slice, uint64_t hartid);

/* Gets the processes that should run on _hartid_ at time _time_. Returns true
 * if a process was found. */
static bool sched_get_proc(uint64_t hartid, uint64_t time, Proc **proc,
                           uint64_t *length);

/* Try to acquire a process, settings its state to RUNNING. If successful,
 * returns true and sets current to proc */
static inline bool sched_acquire_proc(Proc *proc);
static inline void sched_release_proc(void);

static inline bool sched_update(uint64_t begin, uint64_t end, uint64_t mask,
                                uint64_t expected, uint64_t desired,
                                CapNode *cn);

/**************** Definitions ****************/
uint64_t sched_entry(uint64_t s, uint64_t hartid) {
        kassert(MIN_HARTID <= hartid && hartid <= MAX_HARTID);
        s >>= (hartid - MIN_HARTID) * 16;
        return s & 0xFFFF;
}

uint64_t sched_pid(uint64_t s, uint64_t hartid) {
        return sched_entry(s, hartid) & 0xFF;
}

uint64_t sched_depth(uint64_t s, uint64_t hartid) {
        return (sched_entry(s, hartid) >> 8) & 0xFF;
}

bool sched_get_proc(uint64_t hartid, uint64_t time, Proc **proc,
                    uint64_t *length) {
        kassert(MIN_HARTID <= hartid && hartid <= MAX_HARTID);

        /* Set proc to NULL as default */
        *proc = NULL;
        /* Calculate the current quantum */
        uint64_t quantum = time % N_QUANTUM;
        /* Get the current quantum schedule */
        __sync_synchronize();
        uint64_t sched_slice = schedule[quantum];
        uint64_t pid = sched_pid(sched_slice, hartid);

        /* Check if slot is invalid/inactive */
        if (pid & 0x80)
                return true;

        /* Check if some other thread preempts */
        for (size_t i = MIN_HARTID; i < hartid; i++) {
                /* If the pid is same, there is hart with higher priortiy */
                uint64_t other_pid = sched_pid(sched_slice, i);
                if (pid == other_pid)
                        return false;
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
        return true;
}

void sched_release_proc(void) {
        /* Release the process */
        current->state = PROC_SUSPENDED;
        __sync_synchronize();
        if (current->halt)
                __sync_val_compare_and_swap(&current->state, PROC_SUSPENDED,
                                            PROC_HALTED);
        current = 0;
}

bool sched_acquire_proc(Proc *proc) {
        if (__sync_bool_compare_and_swap(&proc->state, PROC_SUSPENDED,
                                         PROC_RUNNING)) {
                current = proc;
                return true;
        }
        return false;
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
        /* Release a process we are holding. */
        if (current)
                sched_release_proc();

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
        } while (proc == NULL || !sched_acquire_proc(proc));

        /* Wait for time slice to start and set timeout */
        wait_and_set_timeout(time, length);
}

bool sched_update(uint64_t begin, uint64_t end, uint64_t mask,
                  uint64_t expected, uint64_t desired, CapNode *cn) {
        for (int i = begin; i < end; i++) {
                if (cn->prev == NULL)
                        return false;
                uint64_t sched_slice = schedule[i];
                if ((sched_slice & mask) != expected)
                        break;
                uint64_t new_sched_slice = (sched_slice & ~mask) | desired;
                __sync_val_compare_and_swap(&schedule[i], sched_slice,
                                            new_sched_slice);
        }
        return true;
}

bool SchedRevoke(const Cap cap, CapNode *cn) {
        uint64_t hartid = cap_time_get_hartid(cap);
        uint64_t begin = cap_time_get_begin(cap);
        uint64_t end = cap_time_get_end(cap);

        uint64_t depth = cap_time_get_depth(cap);
        uint64_t pid = cap_time_get_pid(cap);

        uint64_t mask = 0xFFFF << ((hartid - MIN_HARTID) * 16);
        uint64_t desired = (depth << 8 | pid) << ((hartid - MIN_HARTID) * 16);
        for (int i = begin; i < end; i++) {
                if (cn->prev == NULL)
                        return false;
                uint64_t sched_slice = schedule[i];
                if (sched_depth(sched_slice, hartid) <= depth)
                        continue;
                uint64_t new_sched_slice = (sched_slice & ~mask) | desired;
                __sync_val_compare_and_swap(&schedule[i], sched_slice,
                                            new_sched_slice);
        }
        return true;
}

bool SchedUpdate(const Cap cap, const Cap new_cap, CapNode *cn) {
        kassert(cap_get_type(cap) == CAP_TIME);
        uint64_t hartid = cap_time_get_hartid(new_cap);
        uint64_t begin = cap_time_get_begin(new_cap);
        uint64_t end = cap_time_get_end(new_cap);

        uint64_t old_depth = cap_time_get_depth(cap);
        uint64_t old_pid = cap_time_get_pid(cap);

        uint64_t new_depth = cap_time_get_depth(cap);
        uint64_t new_pid = cap_time_get_pid(cap);

        /* Expected value */
        uint64_t expected = (old_depth << 8 | old_pid)
                            << ((hartid - MIN_HARTID) * 16);
        /* Desired value */
        uint64_t desired = (new_depth << 8 | new_pid)
                           << ((hartid - MIN_HARTID) * 16);
        /* Mask so we match on desired entry */
        uint64_t mask = 0xFFFF << ((hartid - MIN_HARTID) * 16);
        return sched_update(begin, end, mask, expected, desired, cn);
}

bool SchedDelete(const Cap cap, CapNode *cn) {
        kassert(cap_get_type(cap) == CAP_TIME);
        uint64_t tid = cap_time_get_depth(cap);
        uint64_t pid = cap_time_get_pid(cap);
        uint64_t begin = cap_time_get_begin(cap);
        uint64_t end = cap_time_get_end(cap);
        uint64_t hartid = cap_time_get_hartid(cap);
        /* Expected value */
        uint64_t expected = (tid << 8 | pid) << ((hartid - MIN_HARTID) * 16);
        /* Desired value, set pid to invalid/inactive */
        uint64_t desired = 0x80ull << ((hartid - MIN_HARTID) * 16);
        /* Mask so we match on desired entry */
        uint64_t mask = 0xFFFF << ((hartid - MIN_HARTID) * 16);
        return sched_update(begin, end, mask, expected, desired, cn);
}
