// See LICENSE file for copyright and license details.
#pragma once

#include "config.h"
#include "types.h"
#include "proc.h"

uintptr_t AsmAcquireProc(Process *proc);
void AsmReleaseProc(Process *proc);
void sched(void) __attribute__((noreturn));
void sched_init(void);

typedef struct sched_entry {
        uint8_t pid;
        uint8_t fuel;
} SchedEntry;

typedef struct scheduler {
        SchedEntry time_slots[N_QUANTUM][N_CORES];
} Scheduler;

