// See LICENSE file for copyright and license details.
#pragma once

/* Number of cores. */
#define N_CORES 2
/* Number of processes. */
#define N_PROC 4
/* Number of capabilities per process */
#define N_CAPS 256

/* Ticks per second */
#define TICKS_PER_SECOND 10000000UL

/* Number of time slices in a major frame. */
#define N_QUANTUM 256
/* Number of ticks per quantum. */
#define TICKS (TICKS_PER_SECOND/N_QUANTUM)
/* Number of slack ticks (buffer) for scheduler. */
#define SLACK_TICKS (TICKS/10)

/* Stack size. */
#define STACK_SIZE (1024*2)
/* log_2 of stack size. */
#define LOG_STACK_SIZE 11

/* Beginning and end of the user processes' memory region.
 * Used for setting up initial memory of the boot process. */
#define USER_MEMORY_BEGIN 0x80000000UL
#define USER_MEMORY_END 0x100000000UL
#define BOOT_PMP_LENGTH 0x1000UL

#define N_CHANNELS 256

#define SCHEDULE_BENCHMARK 1
/* Currently 1 round = 1 quantum */
#define BENCHMARK_DURATION (TICKS_PER_SECOND*10)
#define BENCHMARK_ROUNDS (BENCHMARK_DURATION / TICKS)