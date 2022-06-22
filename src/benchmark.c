// See LICENSE file for copyright and license details.
#include <stdio.h>

#include "benchmark_util.h"
#include "config.h"
#include "proc.h"

void incremental_benchmark_step() {
        printf("\nValue=%lu\n", current->args[0]);
        current->args[0] = 0;
}

void end_incremental_benchmark(uint64_t duration_recorded) {
        print_relevant_config();
        printf("Duration test ran for:                       %lu\n", duration_recorded);
        printf("Amount of time longer than minimum duration: %lu\n", (duration_recorded - BENCHMARK_DURATION));
        printf("DONE\n");
        while(1)
                ;
}

void end_non_incremental_benchmark(uint64_t duration_recorded) {
        print_relevant_config();
        printf("\nValue=%lu\n", current->args[0]);
        printf("Duration test ran for:                       %lu\n", duration_recorded);
        printf("Amount of time longer than minimum duration: %lu\n", (duration_recorded - BENCHMARK_DURATION));
        printf("DONE\n");
        while(1)
                ;
}