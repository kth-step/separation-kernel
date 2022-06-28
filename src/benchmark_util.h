// See LICENSE file for copyright and license details.
#pragma once

#include <stdio.h>

#include "config.h"

void print_relevant_config() {
    printf( "N_CORES=%d\n"
            "N_PROC=%d\n"
            "N_CAPS=%d\n"
            "TICKS_PER_SECOND=%lu\n"
            "N_QUANTUM=%d\n"
            "TICKS=%lu\n"
            "SLACK_TICKS=%lu\n"
            "STACK_SIZE=%d\n"
            "SCHEDULE_BENCHMARK=%d\n"
            "BENCHMARK_DURATION=%lu\n"
            "BENCHMARK_ROUNDS=%lu\n"
            "PERFORMANCE_SCHEDULING=%d\n"
            "CRYPTO_APP=%d\n"
            "TIME_SLOT_LOANING=%d\n"
            , N_CORES, N_PROC, N_CAPS, TICKS_PER_SECOND, N_QUANTUM, TICKS, SLACK_TICKS, STACK_SIZE
            , SCHEDULE_BENCHMARK, BENCHMARK_DURATION, BENCHMARK_ROUNDS, PERFORMANCE_SCHEDULING, CRYPTO_APP, TIME_SLOT_LOANING);
}

void print_all_config() {
    print_relevant_config();
    printf( "LOG_STACK_SIZE=%d\n"
            "USER_MEMORY_BEGIN=0x%lx\n"
            "USER_MEMORY_END=  0x%lx\n"
            "BOOT_PMP_LENGTH=0x%lx\n"
            "N_CHANNELS=%d\n"
            , LOG_STACK_SIZE, USER_MEMORY_BEGIN, USER_MEMORY_END, BOOT_PMP_LENGTH, N_CHANNELS);
}
