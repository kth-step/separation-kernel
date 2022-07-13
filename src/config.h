// See LICENSE file for copyright and license details.
#pragma once

/* 1 if running code through QEMU, 0 if running through HiFive Unleashed board. 
   If left at 1 here it is automatically set to 0 (and then set back to 1 afterwards) when running the appropriate make command. */
#define QEMU_DEBUGGING 1

#if QEMU_DEBUGGING == 0
    /* Number of cores when running on the board. */
    #define N_CORES 4 
#else
    /* Number of cores when running in QEMU. */
    #define N_CORES 2
#endif 
/* Number of processes. */
#define N_PROC 4
/* Number of capabilities per process */
#define N_CAPS 256
/* Number of PMP registers in hardware */
#define N_PMP 8

/* Ticks per second */
#if QEMU_DEBUGGING == 0
    /* "The CPU real time clock (rtcclk) runs at 1 MHz" https://static.dev.sifive.com/FU540-C000-v1.0.pdf */
    #define TICKS_PER_SECOND 1000000UL
#else
    /* Ticks per second as approximated in QEMU */
    #define TICKS_PER_SECOND 10000000UL
#endif

/* Number of time slices in a major frame. */
#define N_QUANTUM 1
/* Number of ticks per quantum. */
#define TICKS 2000UL
/* Number of slack ticks (buffer) for scheduler. */
#define SLACK_TICKS 200UL

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
#define BENCHMARK_DURATION (TICKS_PER_SECOND*0)
/* We get two rounds less than specified because we need to discard the boot round and the very first measure is before having run any round at all.
   Therefore we explicitly add + 2 to remind us of this (if we want a specific number of data points that is). */
#define BENCHMARK_ROUNDS (1000000UL + 2) // (BENCHMARK_DURATION / TICKS)

#define PERFORMANCE_SCHEDULING 0
#define CRYPTO_APP 0