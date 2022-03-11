// See LICENSE file for copyright and license details.

#include "inc/types.h"
#include "inc/sched.h"
#include "inc/timer.h"
#include "inc/csr.h"
#include "inc/proc.h"
#include "inc/lock.h"

volatile Scheduler scheduler;

Process* get_proc(uintptr_t hartid, uintptr_t time, uintptr_t *length) {
        uintptr_t q = (time / N_TICKS) % N_QUANTUM;
        SchedEntry se = scheduler.time_slots[q][hartid];
        uint8_t pid = se.pid;
        if (pid & 0x80)
                return 0;
        for (int i = 0; i < hartid; i++) {
                uint8_t pidi = scheduler.time_slots[q][i].pid;
                if (pid == pidi)
                        return 0;
        }
        *length = 1;
        for (int i = q+1; i < N_QUANTUM; i++) {
                SchedEntry sei = scheduler.time_slots[i][hartid];
                if (se.pid != sei.pid)
                        break;
                *length+=1;
        }
        return &processes[pid];
}

void sched(void) {
        Process *proc;
        uintptr_t time;
        uintptr_t length;
        uintptr_t hartid = read_csr(mhartid);
        while(1) {
                time = (read_time() & ~(N_TICKS - 1)) + N_TICKS;
                if (!(proc = get_proc(hartid, time, &length)))
                        continue;
                write_timeout(hartid, time + (N_TICKS * length) - N_SLACK_TICKS);
                while (read_time() < time);
                AsmAcquireProc(proc);
        }
}
