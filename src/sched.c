#include "sched.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "benchmark_util.h"
#include "config.h"
#include "csr.h"
#include "stack.h"

static uint64_t round_number = 0;
/* Used to make sure our structures don't get removed by optimization. */
static volatile int dummy_counter = 0; 

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

void InitSched() {
        for (int i = 0; i < N_QUANTUM; i++) {
                if (i % 2 == 0) {
                        schedule[i] = 0x0000000200010000;
                } else {
                        schedule[i] = 0x0000000100020000;
                }
        }
}

static inline uint64_t sched_get_pid(uint64_t s, uintptr_t hartid) {
        return ((s >> (hartid * 16)) & 0xFF);
}

static inline int sched_is_invalid_pid(int8_t pid) {
        return pid < 0;
}

/* The loop in this function affects the running time of the scheduling, 
so we force its worst time execution when establishing time needed for scheduling. */
static inline int sched_has_priority(uint64_t s, uint64_t pid,
                                     uintptr_t hartid) {
        for (size_t i = 0; i < VIRT_N_CORES - 1; i++) {
                /* If the pid is same, there is hart with higher priortiy */
                uint64_t other_pid = sched_get_pid(s, i); /* Process ID. */
                if (pid == other_pid)
                        /* We keep the if-statement to keep the comparison operation, but we don't care about the outcome. */
                        dummy_counter = 1;
        }
        // While this adds one operation (giving us a slightly worse time) it does allow us to keep the determinism during testing.
        return (hartid == 0);
}

/* The loop in this function affects the running time of the scheduling, 
so we force its worst time execution when establishing time needed for scheduling. */
static inline uint64_t sched_get_length(uint64_t s, uint64_t q,
                                        uintptr_t hartid) {
        uint64_t length = 1;
        uint64_t mask = 0xFFFFUL << (hartid * 8);
        for (uint64_t qi = 0 + 1; qi < N_QUANTUM; qi++) {
                /* If next timeslice has the same pid and tsid, then add to
                 * lenght */
                __sync_synchronize();
                uint64_t si = schedule[qi];
                if ((s ^ si) & mask)
                        /* We keep the if-statement to keep the comparison operation, but we don't care about the outcome. */
                        dummy_counter = 1;
                // length++; We want to rerun scheduling as often as possible for more data points, so we don't want to increase length.
                dummy_counter++;
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
        /* Simulate at least one check in the wait loop */
        if (read_time() < time * TICKS) {
                dummy_counter = 1;
        }
        //while (read_time() < time * TICKS) {
        //}
}

#if SLACK_CYCLE_TEST == 1
void Sched(uint64_t cycle_before) {
#else
void Sched(void) {
#endif
        /* Release a process if we are holding it */
        release_current();

        /* The hart/core id */
        uintptr_t hartid = get_software_hartid();

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
                        #if SLACK_CYCLE_TEST == 1
                                #if INSTRUMENTATION_TEST == 1
                                        uint64_t time_after = read_time();
                                #endif
                                uint64_t cycle_after = read_csr(mcycle);
                                printf("Value= %lu\n", (cycle_after-cycle_before));
                        #else
                                uint64_t time_after = read_time();
                                uint64_t remaining_time = (time * TICKS)- time_after;
                                printf("\nValue= %lu\n", remaining_time);
                        #endif
                        round_number++;
                        if (round_number >= SLACK_TEST_ROUNDS) {
                                print_relevant_config();
                                printf("\nDONE\n");
                                exit(0);
                        }
                        #if INSTRUMENTATION_TEST == 1
                                uint64_t time_after_instrumentation = read_time();
                                uintptr_t instrumentation_overhead = time_after_instrumentation - time_after;
                                printf("\nValue:%lu\n", instrumentation_overhead);
                        #endif

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
