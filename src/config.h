// See LICENSE file for copyright and license details.
#pragma once

/* Import board settings fomr 'bsp/$(BSP)/platform.h' */
#include "platform.h"

/* Number of processes. */
#define N_PROC 5

/* Number of capabilities per process */
#define N_CAPS 128

/* Number of time slices in a major frame. */
#define N_QUANTUM 128

/* Number of communications channels */
#define N_CHANNELS 256

/* Number of ticks per quantum. */
/* TICKS_PER_SECOND defined in platform.h */
#define TICKS (TICKS_PER_SECOND / N_QUANTUM)

/* Number of scheduler ticks. */
#define SCHEDULER_TICKS 5000

/* Beginning and end of the user processes' memory region.
 * Used for setting up initial memory of the boot process. */
#define BOOT_PMP_LENGTH 0x100000UL
#define USER_MEMORY_BEGIN 0x80000000UL
#define USER_MEMORY_END 0x100000000UL

#define MEMORY_PROTECTION 1
