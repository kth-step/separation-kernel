#include "sched.h"
#include "stack.h"
#include "lock.h"
#include "csr.h"

uint64_t schedule[N_QUANTUM];

static Process* get_proc(uintptr_t hartid, uint64_t time) {
        uint64_t s = schedule[time % N_QUANTUM];
        uint8_t pid = (s >> (hartid * 16)) & 0xFF;
        if (pid & 0x80)
                return 0;
        for (uintptr_t i = 0; i < hartid; i++) {
                uint8_t other_pid = (s >> (i * 16)) & 0xFF;
                if (other_pid == pid)
                        return 0;
        }
        return &processes[pid];
}

void Sched(void) {
        uintptr_t hartid = read_csr(mhartid);
        uint64_t time, timeout;
        Process *proc;
        if (current) {
                release_lock(&current->lock);
                current = 0;
        }
        while (!current) {
                time = (read_time() + TICKS) & ~(TICKS-1);
                timeout = time + TICKS - SLACK_TICKS;
                proc = get_proc(hartid, time / TICKS);
                if (proc == 0)
                        continue;
                if (try_acquire_lock(&proc->lock))
                        current = proc;
        }
        write_timeout(hartid, timeout);
        while (read_time() < time);
}
