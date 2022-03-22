// See LICENSE file for copyright and license details.
#pragma once
#include <stdint.h>

#include "config.h"
extern uintptr_t proc_stack[N_PROC][STACK_SIZE / 8];
extern uintptr_t core_stack[N_CORES][STACK_SIZE / 8];
