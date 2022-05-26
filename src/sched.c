#include "sched.h"

#include <stddef.h>

#include "csr.h"
#include "lock.h"
#include "stack.h"

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

static inline uint64_t sched_get_pid(uint64_t s, uintptr_t hartid) {
        return ((s >> (hartid * 16)) & 0xFF);
}

static inline int sched_is_invalid_pid(int8_t pid) {
        return pid < 0;
}

static inline int sched_has_priority(uint64_t s, uint64_t pid,
                                     uintptr_t hartid) {
        for (size_t i = 0; i < hartid; i++) {
                /* If the pid is same, there is hart with higher priortiy */
                uint64_t other_pid = sched_get_pid(s, i); /* Process ID. */
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

static int sched_get_proc(uintptr_t hartid, uint64_t time, Proc **proc) {
        /* Calculate the current quantum */
        uint64_t q = time % N_QUANTUM;
        /* Get the current quantum schedule */
        __sync_synchronize();
        uint64_t s = schedule[q];
        uint64_t pid = sched_get_pid(s, hartid); /* Process ID. */
        /* If msb is 1, return 0 */
        if (pid & 0x80)
                return 0;
        /* Check that no other hart with higher priority schedules this pid */
        if (!sched_has_priority(s, pid, hartid))
                return 0;
        /* Set process */
        *proc = &processes[pid];
        /* Return the scheduling length */
        return sched_get_length(s, q, hartid);
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
        while (read_time() < time * TICKS) {
        }
}

void Sched(void) {
        /* Release a process if we are holding it */
        release_current();

        /* The hart/core id */
        uintptr_t hartid = read_csr(mhartid);

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

static inline void sched_update_rev(uint8_t begin, uint8_t end, uint8_t hartid,
                                    uint16_t expected, uint16_t desired) {
        uint64_t mask = 0xFFFF << (hartid * 16);
        uint64_t expected64 = expected << (hartid * 16);
        uint64_t desired64 = desired << (hartid * 16);
        for (int i = begin; i >= end; i--) {
                uint64_t s = schedule[i];
                if ((s & mask) != expected64)
                        break;
                uint64_t s_new = (s & ~mask) | desired64;
                __sync_val_compare_and_swap(&schedule[i], s, s_new);
        }
}
static inline void sched_update(uint8_t begin, uint8_t end, uint8_t hartid,
                                uint16_t expected, uint16_t desired) {
        uint64_t mask = 0xFFFF << (hartid * 16);
        uint64_t expected64 = expected << (hartid * 16);
        uint64_t desired64 = desired << (hartid * 16);
        for (int i = begin; i <= end; i++) {
                uint64_t s = schedule[i];
                if ((s & mask) != expected64)
                        break;
                uint64_t s_new = (s & ~mask) | desired64;
                __sync_val_compare_and_swap(&schedule[i], s, s_new);
        }
}
void SchedUpdate(uint8_t begin, uint8_t end, uint8_t hartid, uint16_t expected,
                 uint16_t desired) {
        if (begin > end)
                sched_update_rev(begin, end, hartid, expected, desired);
        else
                sched_update(begin, end, hartid, expected, desired);
}

