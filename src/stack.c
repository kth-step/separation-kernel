// See LICENSE file for copyright and license details.
#include "stack.h"
uintptr_t proc_stack[N_PROC][PROC_STACK_SIZE / 8];
uintptr_t core_stack[N_CORES][CORE_STACK_SIZE / 8];
