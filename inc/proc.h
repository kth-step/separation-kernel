// See LICENSE file for copyright and license details.

#pragma once

#include "config.h"
#include "types.h"
#include "capabilities.h"
#include "lock.h"

// Struct types {{{
typedef struct trap_registers {
        uintptr_t prevPc;
        uintptr_t prevSp;
        uintptr_t trapVector;
        uintptr_t trapSp;
        uintptr_t trapCause;
        uintptr_t trapValue;
} TrapRegisters;

typedef struct pmp_config {
        uintptr_t pmpcfg0;
        uintptr_t pmpaddr0;
        uintptr_t pmpaddr1;
        uintptr_t pmpaddr2;
        uintptr_t pmpaddr3;
        uintptr_t pmpaddr4;
        uintptr_t pmpaddr5;
        uintptr_t pmpaddr6;
        uintptr_t pmpaddr7;
} PmpConfig;

typedef struct process {
        uintptr_t pid;
        TrapRegisters *trapRegisters;
        PmpConfig pmpConfig;
        Lock lock;
        Capability **capabilities;
        uintptr_t *kernelSP;
        uintptr_t *userPC;
} Process;

extern Process processes[N_PROC];
extern uintptr_t stack[N_PROC][STACK_SIZE];

// }}}
