// See LICENSE file for copyright and license details.
#pragma once

/* 1 if running code through QEMU, 0 if running through HiFive Unleashed board. 
   If left at 1 here it is automatically set to 0 (and then set back to 1 afterwards) when running the appropriate make command. */
#define QEMU_DEBUGGING 1

#if QEMU_DEBUGGING == 0
    /* Number of cores. */
    #define N_CORES 4 
#else
    /* Number of cores. */
    #define N_CORES 2
#endif 
/* Number of processes. */
#define N_PROC 4
/* Number of capabilities per process */
#define N_CAPS 256
/* Number of PMP registers in hardware */
#define N_PMP 8

/* Ticks per second */
// TODO: check what this is on the actual board
#define TICKS_PER_SECOND 10000000UL

/* Number of time slices in a major frame. */
#define N_QUANTUM 256
/* Number of ticks per quantum. */
#define TICKS 20000UL
/* Number of slack ticks (buffer) for scheduler. */
#define SLACK_TICKS 10000UL

/* Stack size. */
#define STACK_SIZE (1024*2)
/* log_2 of stack size. */
#define LOG_STACK_SIZE 11

/* Beginning and end of the user processes' memory region.
 * Used for setting up initial memory of the boot process. */
#define USER_MEMORY_BEGIN 0x80000000UL
#define USER_MEMORY_END  0x100000000UL
#define BOOT_PMP_LENGTH 0x1000UL

#define N_CHANNELS 256

#if QEMU_DEBUGGING == 0
    #define MIN_HARTID 1
    #define MAX_HARTID 4
#else 
    #define MIN_HARTID 0
    #define MAX_HARTID (N_PROC - 1)
#endif

#define SCHEDULE_BENCHMARK 1
/* Currently 1 round = 1 quantum, and the duration consequently assumes a process is only scheduled for one quantum. */
#define BENCHMARK_DURATION (TICKS_PER_SECOND*10)
#define BENCHMARK_ROUNDS (BENCHMARK_DURATION / TICKS)

#define PERFORMANCE_SCHEDULING 1
#define CRYPTO_APP 0