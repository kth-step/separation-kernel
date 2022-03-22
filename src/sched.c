#include "sched.h"
#include "stack.h"
#include "lock.h"
#include "csr.h"

/** The schedule.
 * Each 64-bit word describes the state of four cores:
 *   [(pid,cap), (pid,cap), (pid,cap), (pid,cap)]
 * each pid, cap is the index to the capability table are 8 bit wide 
 * and describes the process to run and the corresponding capability 
 * used. If the msb of bit is 1, then we have no process to run.
 *
 * We should probably replace uint64_t with appropriate structs. 
 */
volatile uint64_t schedule[N_QUANTUM];

/* Get the pid of a hart from schedule s */
static uint64_t get_spid(uint64_t s, uintptr_t hartid) {
        return (s >> (hartid * 16)) & 0xFF;
}

/* Get the capability index of a hart from schedule s */
static uint64_t get_scap(uint64_t s, uintptr_t hartid) {
        return (s >> (hartid * 16 + 8)) & 0xFF;
}

/* Get the entry for a hart. */
static uint64_t get_sentry(uint64_t s, uintptr_t hartid) {
        return (s >> (hartid * 16)) & 0xFFFF;
}

static int get_proc(uintptr_t hartid, uint64_t time, Process **proc, int *length) {
        /* Calculate the current quantum */
        int q = (time/TICKS) % N_QUANTUM;
        /* Get the current quantum schedule */
        uint64_t s = schedule[q];
        /* If msb is 1, return 0 */
        if (get_spid(s,hartid) & 0x80)
                return 0;
        /* Check that no other hart with higher priority schedules this pid */
        for (int i = 0; i < hartid; i++) {
                /* If the pid is same, there is hart with higher priortiy */
                if (get_spid(s,hartid) == get_spid(s,i))
                        return 0;
        }
        /* Calculate the length of the timeslice */
        *length = 1;
        for (int i = q+1; i < N_QUANTUM; i++) {
                /* If next timeslice has the same pid and cap, then add to lenght */
                uint64_t si = schedule[i];
                if (get_sentry(s,hartid) == get_sentry(si,hartid)) 
                        *length++;
        }
        *proc = processes + get_spid(s,hartid);
        /* Return success */
        return 1;
}

static void release_current(void) {
        /* If current == 0, we have nothing to release */
        if (current) {
                /* Release the process */
                release_lock(&current->lock);
                current = 0;
        }
}

void Sched(void) {
        /* Release a process if we are holding it */
        release_current();

        /* The hart/core id */
        uintptr_t hartid = read_csr(mhartid);

        /* Start of next time slice */
        uint64_t time;

        /* Process to run and number of time slices to run for */
        Process *proc;
        int length;

        /* Here the core tries to fetch a process to run */
        while (!current) {
                /* Get the start of next time slice. */
                time = (read_time() + TICKS) & ~(TICKS-1);
                /* Try getting a process at that time slice. */
                if (get_proc(hartid, time, &proc, &length) 
                                && try_acquire_lock(&proc->lock)) {
                        current = proc;
                }
        }

        /* Timeout.
         * TICKS * length is the number of ticks for process to run of which
         * SLACK_TIME is the time reserved for scheduler.
         */
        uint64_t timeout = time + TICKS * length - SLACK_TICKS;

        /* Write to timeout register */
        write_timeout(hartid, timeout);
        /* Wait until it is time to run */
        while (read_time() < time);
        /* We should return to AsmSwitchToProc. */
}
