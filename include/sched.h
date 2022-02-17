// See LICENSE file for copyright and license details.

#pragma once

#include "config.h"
#include "types.h"

typedef struct sched_entry {
        uint8_t pid;
        uint8_t fuel;
} SchedEntry;

typedef struct scheduler {
        SchedEntry time_slots[N_QUANTUM][N_CORES];
} Scheduler;

