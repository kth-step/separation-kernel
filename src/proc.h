// See LICENSE file for copyright and license details.
#pragma once

#include "types.h"
#include "config.h"
#include "capabilities.h"
#include "lock.h"

typedef struct process {
        uintptr_t *ksp;
        uintptr_t pid;
        Capability **capabilities;
        uintptr_t args[8];
        uintptr_t *pc;
        uintptr_t lock;
} Process;

extern Process processes[N_PROC];
register Process *current __asm__("tp");
