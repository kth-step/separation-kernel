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
/* Used when testing scheduling; we don't actually run with something else than N_CORES, 
   but we want the scheduler to be able to think that we run with another amount of cores 
   to affect the time it takes to complete the scheduling. 
   Since the separation kernel currently only supports up to 4 cores, it is assumed that this value does not surpass 4 either.*/
#define VIRT_N_CORES 4
/* Number of processes. */
#define N_PROC 4
/* Number of capabilities per process */
#define N_CAPS 256
/* Number of PMP registers in hardware */
#define N_PMP 8

/* Ticks per second */
/* "The CPU real time clock (rtcclk) runs at 1 MHz" https://static.dev.sifive.com/FU540-C000-v1.0.pdf */
#define TICKS_PER_SECOND 1000000UL

/* Number of time slices in a major frame. */
#define N_QUANTUM 256
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

#define INSTRUMENTATION_TEST 0
/* While some test are quite intrusive in the correctness of the separation kernel the cycle test is parhaps even more so than the other ones. 
It should definitely be used with care; make sure to check all places with if-guards for this value and make sure it does not 
break something you depend on. */
#define SLACK_CYCLE_TEST 0
/* We get one round less than specified for both slack testing and instrumentation testing,
   because we need to discard the boot round for slack, and because we quit before measuring in the last round for instrumentation.
   Therefore we explicitly add + 1 to remind us of this. */
#define SLACK_TEST_ROUNDS (1000000 + 1)