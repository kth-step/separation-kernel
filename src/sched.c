// See LICENSE file for copyright and license details.
#include "sched.h"

#include <stddef.h>

#include "csr.h"
#include "lock.h"
#include "stack.h"
#include "cap.h"

/** The schedule.
 * Each 64-bit word describes the state of four cores:
 *   [(pid,cap), (pid,cap), (pid,cap), (pid,cap)]
 * each pid, cap is the index to the capability table are 8 bit wide
 * and describes the process to run and the corresponding capability
 * used. If the msb of bit is 1, then we have no process to run.
 *
 * We should probably replace uint64_t with appropriate structs.
 */
uint64_t schedule[N_QUANTUM];

static inline uint64_t sched_tid(uint64_t entry, uint64_t core) {
        kassert(core < N_CORES);
        entry >>= core * 16;
        return (entry >> 8) & 0xFF;
}

static inline uint64_t sched_pid(uint64_t entry, uint64_t core) {
        kassert(core < N_CORES);
        entry >>= core * 16;
        return entry & 0x7F;
}

static inline bool sched_invalid(uint64_t entry, uint64_t core) {
        kassert(core < N_CORES);
        entry >>= core * 16;
        return entry & 0x80;
}

static inline int sched_has_priority(uint64_t s, uint64_t pid,
                                     uintptr_t hartid) {
        for (size_t i = 0; i < hartid; i++) {
                /* If the pid is same, there is hart with higher priortiy */
                uint64_t other_pid = sched_pid(s, i); /* Process ID. */
                if (pid == other_pid)
                        return 0;
        }
        return 1;
}

static inline uint64_t sched_get_length(uint64_t s, uint64_t q,
                                        uintptr_t hartid) {
        uint64_t length = 1;
        uint64_t mask = 0xFFFFUL << (hartid * 8);
        for (uint64_t qi = q + 1; qi < N_QUANTUM; qi++) {
                /* If next timeslice has the same pid and tsid, then add to
                 * lenght */
                __sync_synchronize();
                uint64_t si = schedule[qi];
                if ((s ^ si) & mask)
                        break;
                length++;
        }
        return length;
}

static int sched_get_proc(uintptr_t core, uint64_t time, Proc **proc) {
        /* Calculate the current quantum */
        uint64_t q = time % N_QUANTUM;
        /* Get the current quantum schedule */
        __sync_synchronize();
        uint64_t s = schedule[q];
        /* If msb is 1, return 0 */
        if (sched_invalid(s, core))
                return 0;

        uint64_t pid = sched_pid(s, core); /* Process ID. */
        /* Check that no other hart with higher priority schedules this pid */
        if (!sched_has_priority(s, pid, core))
                return 0;
        /* Set process */
        *proc = &processes[pid];
        /* Return the scheduling length */
        return sched_get_length(s, q, core);
}

static void release_current(void) {
        /* If current == 0, we have nothing to release */
        if (current) {
                /* Release the process */
                current->state = PROC_SUSPENDED;
                __sync_synchronize();
                if (current->halt)
                        __sync_val_compare_and_swap(
                            &current->state, PROC_SUSPENDED, PROC_HALTED);
                current = 0;
        }
}

bool sched_acquire_proc(Proc *proc) {
        current = proc;
        return __sync_bool_compare_and_swap(&proc->state, PROC_SUSPENDED,
                                            PROC_RUNNING);
}

void set_timeout(uint64_t quantum) {
        /* Timeout.
         * TICKS * length is the number of ticks for process to
         * run of which SLACK_TIME is the time reserved for
         * scheduler.
         */
        uint64_t timeout = quantum * TICKS - SLACK_TICKS;
        write_timeout(read_csr(mhartid), timeout);
}

void wait(uint64_t time) {
        while (read_time() < time * TICKS);
}

void Sched(void) {
        /* Release a process if we are holding it */
        release_current();

        /* The hart/core id */
        uintptr_t hartid = read_csr(mhartid) - MIN_HARTID;

        /* Process to run and number of time slices to run for */
        Proc *proc;

        /* Here the core tries to fetch a process to run */
        while (1) {
                /* Get the start of next time slice. */
                uint64_t time = (read_time() / TICKS) + 1;
                /* Try getting a process at that time slice. */
                uint64_t length = sched_get_proc(hartid, time, &proc);
                if (length > 0 && sched_acquire_proc(proc)) {
                        /* Set timeout */
                        set_timeout(time + length);
                        /* Wait until it is time to run */
                        wait(time);
                        /* Returns to AsmSwitchToProc. */
                        return;
                }
        }
}

static inline bool sched_update(uint64_t begin, uint64_t end, uint64_t mask,
                                uint64_t expected, uint64_t desired, CapNode *cn) {
        for (int i = begin; i < end; i++) {
                if (cn->prev == NULL)
                        return false;
                uint64_t s = schedule[i];
                if ((s & mask) != expected)
                        break;
                uint64_t s_new = (s & ~mask) | desired;
                __sync_val_compare_and_swap(&schedule[i], s, s_new);
        }
        return true;
}

bool SchedRevoke(const Cap cap, CapNode *cn) {
        uint64_t core = cap_time_get_core(cap);
        uint64_t begin = cap_time_get_begin(cap);
        uint64_t end = cap_time_get_end(cap);

        uint64_t depth = cap_time_get_depth(cap);
        uint64_t pid = cap_time_get_pid(cap);

        uint64_t mask = 0xFFFF << (core * 16);
        uint64_t desired = (depth << 8 | pid) << (core * 16);
        for (int i = begin; i < end; i++) {
                if (cn->prev == NULL)
                        return false;
                uint64_t s_expected = schedule[i];
                if (sched_tid(s_expected, core) <= depth)
                        continue;
                uint64_t s_desired = (s_expected & ~mask) | desired;
                __sync_val_compare_and_swap(&schedule[i], s_expected, s_desired);
        }
        return true;
}

bool SchedUpdate(const Cap cap, const Cap new_cap, CapNode *cn) {
        kassert(cap_get_type(cap) == CAP_TIME);
        uint64_t core = cap_time_get_core(new_cap);
        uint64_t begin = cap_time_get_begin(new_cap);
        uint64_t end = cap_time_get_end(new_cap);

        uint64_t old_depth = cap_time_get_depth(cap);
        uint64_t old_pid = cap_time_get_pid(cap);

        uint64_t new_depth = cap_time_get_depth(cap);
        uint64_t new_pid = cap_time_get_pid(cap);

        uint64_t expected = (old_depth << 8 | old_pid) << (core * 16);
        uint64_t desired = (new_depth << 8 | new_pid) << (core * 16);
        uint64_t mask = 0xFFFF << (core * 16);
        return sched_update(begin, end, mask, expected, desired, cn);
}

bool SchedDelete(const Cap cap, CapNode *cn) {
        kassert(cap_get_type(cap) == CAP_TIME);
        uint64_t tid = cap_time_get_depth(cap);
        uint64_t pid = cap_time_get_pid(cap);
        uint64_t begin = cap_time_get_begin(cap);
        uint64_t end = cap_time_get_end(cap);
        uint64_t core = cap_time_get_core(cap);
        uint64_t expected = (tid << 8 | pid) << (core * 16);
        uint64_t desired = 0x80ull << (core * 16);
        uint64_t mask = 0xFFFF << (core * 16);
        return sched_update(begin, end, mask, expected, desired, cn);
}
