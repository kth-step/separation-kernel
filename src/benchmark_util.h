// See LICENSE file for copyright and license details.
#pragma once

#include <stdio.h>

#include "config.h"

void print_relevant_config() {
    printf( "N_CORES=%d\n"
            "VIRT_N_CORES=%d\n"
            "N_PROC=%d\n"
            "N_CAPS=%d\n"
            "TICKS_PER_SECOND=%lu\n"
            "N_QUANTUM=%d\n"
            "TICKS=%lu\n"
            "SLACK_TICKS=%lu\n"
            "STACK_SIZE=%d\n"
            "INSTRUMENTATION_TEST=%d\n"
            "SLACK_TEST_ROUNDS=%d\n"
            "SLACK_CYCLE_TEST=%d\n"
            "QEMU_DEBUGGING=%d\n"
            , N_CORES, VIRT_N_CORES, N_PROC, N_CAPS, TICKS_PER_SECOND, N_QUANTUM, TICKS, SLACK_TICKS, STACK_SIZE
            , INSTRUMENTATION_TEST, SLACK_TEST_ROUNDS, SLACK_CYCLE_TEST, QEMU_DEBUGGING);
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
