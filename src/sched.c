#include "sched.h"

#include <stddef.h>
#include <stdlib.h>

#include "config.h"
#include "csr.h"
#include "lock.h"
#include "stack.h"

#if SCHEDULE_BENCHMARK == 1
        extern void end_incremental_benchmark();
        extern void incremental_benchmark_step();
        static int round_counter = 0;
#endif

/** The schedule.
 * Each 64-bit word describes the state of four cores:
 *   [(pid,cap), (pid,cap), (pid,cap), (pid,cap)]
 * each pid, cap is the index to the capability table are 8 bit wide
 * and describes the process to run and the corresponding capability
 * used. If the msb of bit is 1, then we have no process to run.
 *
 * We should probably replace uint64_t with appropriate structs.
 */
static uint64_t schedule[N_QUANTUM];
static Lock lock = 0;

#if TIME_SLOT_LOANING != 0
        TimeSlotInstanceRoot time_slot_instance_roots[N_QUANTUM][N_CORES];
#endif

void InitSched() {
        for (int i = 0; i < N_QUANTUM; i++) {
                if (i % 2 == 0) {
                        schedule[i] = 0x0000000200010000;
                } else {
                        schedule[i] = 0x0000000100020000;
                }
        }
}

#if TIME_SLOT_LOANING != 0
        void InitTimeSlotInstanceRoots() {
                for (int q = 0; q < N_QUANTUM; q++) {
                        for (int hartid = 0; hartid < N_CORES; hartid++) {
                                time_slot_instance_roots[q][hartid].pidp = ((uint8_t *)(schedule + q)) + (hartid * 2);
                                time_slot_instance_roots[q][hartid].head = NULL;
                        }
                }
        }

        TimeSlotInstance * SetLoanedTimeSlot(uint64_t quantum, uint64_t hartid, uint8_t pid) {
                TimeSlotInstanceRoot * root = &time_slot_instance_roots[quantum][hartid];
                TimeSlotInstance * prev_head = root->head;
                TimeSlotInstance * new_instance = (TimeSlotInstance *)malloc(sizeof(TimeSlotInstance));
                if (new_instance == NULL) return NULL;
                new_instance->pid = pid;
                new_instance->loaner = prev_head;
                root->head = new_instance;
                return new_instance;
        }

        int RevokeLoanedTimeSlot(uint64_t quantum, uint64_t hartid, TimeSlotInstance * instance) {
                TimeSlotInstanceRoot * root = &time_slot_instance_roots[quantum][hartid];
                TimeSlotInstance * next = root->head;
                if (next == instance) {
                        root->head = next->loaner;
                        free(next);
                        return 1;
                } else if (next == NULL) {
                        return 0;
                }

                next = next->loaner;
                while (next != NULL) {
                        if (next == instance) {
                                TimeSlotInstance * curr = root->head;
                                root->head = next->loaner;
                                while (curr != root->head) {
                                        next = curr->loaner;
                                        free(curr);
                                        curr = next;
                                }
                                return 1;
                        }
                        next = next->loaner;
                }
                return 0;
        }

        int ReleaseCurrentTimeSlot() {
                uint64_t q = (read_time() / TICKS) % N_QUANTUM;
                uint64_t hartid = read_csr(mhartid);
                TimeSlotInstance * curr = time_slot_instance_roots[q][hartid].head;
                if (curr == NULL) return 0;
                time_slot_instance_roots[q][hartid].head = curr->loaner;
                free(curr);
                return 1;
        }

        static uint64_t sched_get_time_slot_pid(uint64_t quantum, uint64_t hartid) {
                TimeSlotInstanceRoot * root = &time_slot_instance_roots[quantum][hartid];
                TimeSlotInstance * head = root->head;
                if (head == NULL) return *(root->pidp);
                else return head->pid;
        }
#endif

static inline uint64_t sched_get_tid(uint64_t s, uintptr_t hartid) {
        return ((s >> (hartid * 16)) & 0xFF00);
}

static inline uint64_t sched_get_pid(uint64_t s, uintptr_t hartid) {
        return ((s >> (hartid * 16)) & 0xFF);
}

static inline int sched_is_invalid_pid(int8_t pid) {
        return pid < 0;
}

#if PERFORMANCE_SCHEDULING == 0
static inline int sched_has_priority(uint64_t quantum, uint64_t pid,
                                     uintptr_t hartid) {
        for (size_t i = 0; i < hartid; i++) {
                /* If the pid is same, there is hart with higher priortiy */
                #if TIME_SLOT_LOANING == 0
                        uint64_t other_pid = sched_get_pid(schedule[quantum], i); /* Process ID. */
                #else
                        uint64_t other_pid = sched_get_time_slot_pid(quantum, i);
                #endif
                if (pid == other_pid)
                        return 0;
        }
        return 1;
}
#endif

static inline uint64_t sched_get_length(uint64_t s, uint64_t q,
                                        uintptr_t hartid) {
        uint64_t length = 1;
        uint64_t mask = 0xFFFFUL << (hartid * 16);
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
        #if TIME_SLOT_LOANING == 0
                uint64_t pid = sched_get_pid(s, hartid); /* Process ID. */
        #else 
                uint64_t pid = sched_get_time_slot_pid(q, hartid); /* Process ID. */
        #endif
        /* If msb is 1, return 0 */
        if (pid & 0x80)
                return 0;
        #if PERFORMANCE_SCHEDULING == 0
                /* Check that no other hart with higher priority schedules this pid */
                if (!sched_has_priority(q, pid, hartid))
                        return 0;
        #endif
        /* Set process */
        *proc = &processes[pid];
        /* Return the scheduling length */
        #if SCHEDULE_BENCHMARK == 0
                return sched_get_length(s, q, hartid);
        #endif
        #if SCHEDULE_BENCHMARK == 1
                return (0 != sched_get_length(s, q, hartid));
        #endif
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

void wait_next_scheduling(uint64_t time) {
        while (read_time() < ((time + 1) * TICKS - SLACK_TICKS)) {
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
                #if SCHEDULE_BENCHMARK == 0
                        uint64_t time = (read_time() / TICKS) + 1;
                #else
                        uint64_t time_ticks = read_time();
                        uint64_t time = (time_ticks / TICKS) + 1;
                #endif
                /* Try getting a process at that time slice. */
                uint64_t length = sched_get_proc(hartid, time, &proc);
                if (length > 0 && sched_acquire_proc(proc)) {
                        /* Set timeout */
                        set_timeout(time + length);
                        #if PERFORMANCE_SCHEDULING == 0
                                /* Wait until it is time to run */
                                wait(time);
                        #endif
                        #if SCHEDULE_BENCHMARK != 0
                                incremental_benchmark_step();
                                if (++round_counter >= BENCHMARK_ROUNDS) end_incremental_benchmark(time_ticks);
                        #endif
                        /* Returns to AsmSwitchToProc. */
                        return;
                } else {
                        // To prevent constant attempts to acquire the lock when another hart got it.
                        // While not necessary in the non-performance version, it doesn't hurt either.

                        // TODO: Have not been able to establish that this makes a meaningful difference.
                        // Tested with 4 cores with 3 not getting scheduled, for over 200 quanta in a row, with 20000 TICKS each quanta.
                        // User code was only incrementing arg 0 over and over.
                        wait_next_scheduling(time);
                }
        }
}

static inline bool sched_update(uint64_t begin, uint64_t end, uint8_t hartid,
                                uint16_t expected, uint16_t desired, Cap * c) {
        uint64_t mask = 0xFFFF << (hartid * 16);
        uint64_t expected64 = expected << (hartid * 16);
        uint64_t desired64 = desired << (hartid * 16);
        
        /* Disable preemption 
           This is needed since we might remove our own scheduling, and if we are descheduled before
           completing this function then we might still hold the lock without any chance of being scheduled again. */
        clear_csr(mstatus, 8);
        /* Try acquire lock */
        while (!try_acquire_lock(&lock)) {
                /* If failed acquire lock, enable preemption temporary */
                set_csr(mstatus, 8);
                clear_csr(mstatus, 8);
        }
        /* Check that the capability still exists */
        // TODO: Not sure if this is needed anymore
        if (c->prev == NULL) {
                /* If capability deleted, release lock, enable preemption and return */
                release_lock(&lock);
                set_csr(mstatus, 8);
                return false;
        }
        /* Return false if we don't manage to update a single time slot. */
        bool is_updated = false;
        for (int i = begin; i < end; i++) {
                uint64_t s = schedule[i];
                /* This "continue" is needed since we might have a child in a subset of the region and if we move the parent
                   then it should not update the time slots belonging to the child. 
                   
                   Other than that this also protects against overwriting changes made by a revoke
                   (though a "break" would suffice for this). */
                if ((s & mask) != expected64)
                        continue;
                uint64_t s_new = (s & ~mask) | desired64;
                schedule[i] = s_new;
                is_updated = true;
        }
        /* Release lock and enable preemption */
        release_lock(&lock);
        set_csr(mstatus, 8);
        return is_updated;
}

bool SchedUpdate(uint64_t begin, uint64_t end, uint8_t hartid, uint16_t expected,
                 uint16_t desired, Cap * c) {
        if (begin > end)
                return sched_update(end, begin, hartid, expected, desired, c);
        else
                return sched_update(begin, end, hartid, expected, desired, c);
}


bool SchedRevoke(uint64_t begin, uint64_t end, uint8_t hartid,
                 uint16_t desired, Cap * c) {
        uint64_t mask = 0xFFFF << (hartid * 16);
        uint64_t desired64 = desired << (hartid * 16);

        acquire_lock(&lock);

        /* We need to check that our cap has not been removed. If it has we might be in a situation
           where a parent cap has derived a new cap with lower tsid/depth than this cap and which is 
           overapping with our region. In this case we would incorrectly overwrite this new cap. */
        if (c->prev == NULL) {
                release_lock(&lock);
                return false;
        }

        for (int i = begin; i < end; i++) {
                /* We assume that our cap or its children own all time slots on the given hart in this interval.
                   That is, we assume that we don't risk stealing time slots from parents or siblings. */
                uint64_t sched_slice = schedule[i];
                uint64_t new_sched_slice = (sched_slice & ~mask) | desired64;
                schedule[i] = new_sched_slice;
        }
        release_lock(&lock);
        return true;
}


bool SchedDelete(uint64_t begin, uint64_t end, uint8_t hartid, uint16_t expected, uint16_t desired) {
        uint64_t mask = 0xFFFF << (hartid * 16);
        uint64_t expected64 = expected << (hartid * 16);
        uint64_t desired64 = desired << (hartid * 16);
        
        /* Disable preemption  
           This is needed since we might remove all our own scheduling slots, and if we are descheduled before
           completing this function then we might still hold the lock without any chance of being scheduled again. */
        clear_csr(mstatus, 8);
        /* Try acquire lock */
        while (!try_acquire_lock(&lock)) {
                /* If failed acquire lock, enable preemption temporary */
                set_csr(mstatus, 8);
                clear_csr(mstatus, 8);
        }
        /* Return false if we don't manage to update a single time slot. */
        bool is_updated = false;
        for (int i = begin; i < end; i++) {
                uint64_t s = schedule[i];
                if ((s & mask) != expected64)
                        continue;
                uint64_t s_new = (s & ~mask) | desired64;
                schedule[i] = s_new;
                is_updated = true;
        }
        /* Release lock and enable preemption */
        release_lock(&lock);
        set_csr(mstatus, 8);
        return is_updated;
}

bool SchedDeleteAssumeNoPreemption(uint64_t begin, uint64_t end, uint8_t hartid, uint16_t expected, uint16_t desired) {
        uint64_t mask = 0xFFFF << (hartid * 16);
        uint64_t expected64 = expected << (hartid * 16);
        uint64_t desired64 = desired << (hartid * 16);
        
        /* Try acquire lock */
        while (!try_acquire_lock(&lock)) {
                /* If failed acquire lock, enable preemption temporary */
                set_csr(mstatus, 8);
                clear_csr(mstatus, 8);
        }
        /* Return false if we don't manage to update a single time slot. */
        bool is_updated = false;
        for (int i = begin; i < end; i++) {
                uint64_t s = schedule[i];
                if ((s & mask) != expected64)
                        continue;
                uint64_t s_new = (s & ~mask) | desired64;
                schedule[i] = s_new;
                is_updated = true;
        }
        /* Release lock */
        release_lock(&lock);
        return is_updated;
}
