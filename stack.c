// See LICENSE file for copyright and license details.
#include "inc/stack.h"
uintptr_t proc_stack[N_PROC][STACK_SIZE/8]; 
uintptr_t core_stack[N_CORES][STACK_SIZE/8];
