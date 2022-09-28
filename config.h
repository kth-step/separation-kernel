// See LICENSE file for copyright and license details.
#pragma once

/* Number of processes. */
#define N_PROC 6

/* Number of capabilities per process */
#define N_CAPS 64

/* Number of time slices in a major frame. */
#define N_QUANTUM 128

/* Number of communications channels */
#define N_CHANNELS (N_PROC * (N_PROC - 1))

/* Number of ticks per quantum. */
/* TICKS_PER_SECOND defined in platform.h */
#define TICKS (TICKS_PER_SECOND / N_QUANTUM / 100)

/* Number of scheduler ticks. */
#define SCHEDULER_TICKS 2000

/* Uncomment to enable memory protection */
//#define MEMORY_PROTECTION

/* Set absolute path of payload */
//#define PAYLOAD "my_payload.bin"
