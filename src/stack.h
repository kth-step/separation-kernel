// See LICENSE file for copyright and license details.
#pragma once
#include "config.h"
#include "types.h"
/* The process stack is used by the kernel to service
 * individual processes (e.g., syscalls and exceptions).
 * This stack is necessary for preemption of the syscalls.
 */
extern uintptr_t proc_stack[N_PROC][STACK_SIZE / 8];
/* The core stack is used by the kernel to service
 * core related routines (e.g., scheduler) unrelated to
 * individual processes.
 */
extern uintptr_t core_stack[N_CORES][STACK_SIZE / 8];
