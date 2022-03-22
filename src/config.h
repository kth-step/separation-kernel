// See LICENSE file for copyright and license details.
#pragma once

/* Number of cores. */
#define N_CORES 2
/* Number of processes. */
#define N_PROC 4

/* Number of time slices in a major frame. */
#define N_QUANTUM 64 
/* Number of ticks per quantum. */
#define TICKS (8192)
/* Number of slack ticks (buffer) for scheduler. */
#define SLACK_TICKS (TICKS/16)

/* Stack size. */
#define STACK_SIZE 1024
/* log_2 of stack size. */
#define LOG_STACK_SIZE 10
