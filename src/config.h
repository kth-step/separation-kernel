// See LICENSE file for copyright and license details.
#pragma once

/* Number of cores. */
#define N_CORES 2
/* Number of processes. */
#define N_PROC 4
/* Number of capabilities per process */
#define N_CAPS 256

/* Number of time slices in a major frame. */
#define N_QUANTUM 64
/* Number of ticks per quantum. */
#define TICKS (8192)
/* Number of slack ticks (buffer) for scheduler. */
#define SLACK_TICKS (TICKS / 16)

/* Stack size. */
#define STACK_SIZE 1024
/* log_2 of stack size. */
#define LOG_STACK_SIZE 10

/* Beginning and end of the user processes' memory region. 
 * Used for setting up initial memory of the boot process. */
#define USER_MEMORY_BEGIN 0x80000000UL
#define USER_MEMORY_END 0x100000000UL

/* Boot PC. This will be removed later. */
#ifndef __ASSEMBLER__
extern void user_code();
#define BOOT_PC (uintptr_t*)user_code
#endif
