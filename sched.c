// See LICENSE file for copyright and license details.

#include "types.h"
#include "sched.h"
#include "timer.h"
#include "csr.h"
#include "proc.h"
#include "lock.h"

Scheduler scheduler;

uint8_t get_proc() {
        uintptr_t q = (read_time() / N_TICKS) % N_QUANTUM;
        uintptr_t slice = *((uintptr_t*)scheduler.time_slots[q]);
        uintptr_t hartid = read_csr(mhartid);
        uint8_t pid = (slice >> (hartid * 16)) & 0x3F;
        uintptr_t mask = 0;
        for (int i = 0; i < hartid; i++) {
                uintptr_t pidi = (slice >> (i * 16)) & 0x3F;
                mask |= (1 << pidi);
        }
        return (mask & (1 << pid)) ? 0 : pid;
}

uintptr_t sched(void) {
        Process *proc;
        do {
                uintptr_t pid = get_proc();
                Process *proc = &processes[pid];
        } while (try_acquire_lock(&proc->lock));
        

        return 0;
}
