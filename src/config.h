// See LICENSE file for copyright and license details.
#pragma once
#include "platform.h"

/* Number of processes. */
#define N_PROC 4
/* Number of capabilities per process */
#define N_CAPS 128

/* Number of time slices in a major frame. */
#define N_QUANTUM 128
/* Number of ticks per quantum. */
#define TICKS (TICKS_PER_SECOND / N_QUANTUM)
/* Number of slack ticks (buffer) for scheduler. */
#define SLACK_TICKS (TICKS / 10)

/* Stack size. */
#define PROC_STACK_SIZE 512
#define CORE_STACK_SIZE 128
/* log_2 of stack size. */
#define LOG_PROC_STACK_SIZE 9
#define LOG_CORE_STACK_SIZE 7

/* Beginning and end of the user processes' memory region.
 * Used for setting up initial memory of the boot process. */
#define USER_MEMORY_BEGIN 0x80000000UL
#define USER_MEMORY_END  0x100000000UL
#define BOOT_PMP_LENGTH 0x1000UL

#define N_CHANNELS 256

