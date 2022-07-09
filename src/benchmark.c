// See LICENSE file for copyright and license details.
#include <stdio.h>

#include "config.h"
#include "print_util.h"
#include "proc.h"

void end_benchmark(uint64_t duration_recorded, uint64_t values[]) {
        for (int i = 0; i < BENCHMARK_ROUNDS; i++) {
                printf("Value=%lu\n", values[i]);
        }
        print_relevant_config();
        #if BENCHMARK_DURATION == 0
                printf("Duration test ran for: %lu\n", duration_recorded);
                double sec = ((double)duration_recorded) / ((double)TICKS_PER_SECOND);
                printf("In seconds: %f\n", sec);
                if (sec > 60.0)
                        printf("In minutes (rounded down): %d\n", (int)(sec / 60));
        #else
                printf("Duration test ran for:                        %lu\n", duration_recorded);
                printf("Amount of ticks longer than minimum duration: %lu\n", (duration_recorded - BENCHMARK_DURATION));
                double sec = ((double)duration_recorded) / ((double)TICKS_PER_SECOND);
                printf("Duration ran for in seconds: %f\n", sec);
                if (sec > 60.0)
                        printf("In minutes (rounded down): %d\n", (int)(sec / 60));
        #endif
        printf("DONE\n");
        while(1)
                ;
}