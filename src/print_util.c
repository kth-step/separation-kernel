// See LICENSE file for copyright and license details.
#include "print_util.h"

#include "lock.h"

Lock print_lock = 0;

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
            "IPC_BENCHMARK=%d\n"
            "ONLY_2_PROC_IPC=%d\n"
            #if BENCHMARK_DURATION != 0
                "BENCHMARK_DURATION=%lu\n"
            #endif
            "BENCHMARK_ROUNDS=%lu\n"
            "PERFORMANCE_SCHEDULING=%d\n"
            "TIME_SLOT_LOANING=%d\n"
            "TIME_SLOT_LOANING_SIMPLE=%d\n"
            "CRYPTO_APP=%d\n"
            "CRYPTO_IPC=%d\n"
            "QEMU_DEBUGGING=%d\n"
            , N_CORES, N_PROC, N_CAPS, TICKS_PER_SECOND, N_QUANTUM, TICKS, SLACK_TICKS, STACK_SIZE, SCHEDULE_BENCHMARK, IPC_BENCHMARK, ONLY_2_PROC_IPC
            #if BENCHMARK_DURATION != 0
                , BENCHMARK_DURATION 
            #endif
            , BENCHMARK_ROUNDS, PERFORMANCE_SCHEDULING, TIME_SLOT_LOANING, TIME_SLOT_LOANING_SIMPLE, CRYPTO_APP, CRYPTO_IPC, QEMU_DEBUGGING);
}

void print_relevant_config_locked() {
    acquire_lock(&print_lock);
    print_relevant_config();
    release_lock(&print_lock);
}

void print_all_config() {
    print_relevant_config();
    printf( "N_PMP=%d\n"
            "LOG_STACK_SIZE=%d\n"
            "USER_MEMORY_BEGIN=0x%lx\n"
            "USER_MEMORY_END=  0x%lx\n"
            "BOOT_PMP_LENGTH=0x%lx\n"
            "N_CHANNELS=%d\n"
            "MAX_FUEL=%d\n"
            "MIN_HARTID=%d\n"
            "MAX_HARTID=%d\n"
            , N_PMP, LOG_STACK_SIZE, USER_MEMORY_BEGIN, USER_MEMORY_END, BOOT_PMP_LENGTH
            , N_CHANNELS, MAX_FUEL, MIN_HARTID, MAX_HARTID);
}

void print_all_config_locked() {
    acquire_lock(&print_lock);
    print_all_config();
    release_lock(&print_lock);
}

void print_sched(uint64_t schedule[]) {
    printf("\nSchedule:\n");
    for (size_t i = 0; i < N_QUANTUM; i++) {
        printf("%04lx %04lx %04lx %04lx\n", 
               (schedule[i] >> (16*3)) & 0xFFFF, 
               (schedule[i] >> (16*1)) & 0xFFFF, 
               (schedule[i] >> (16*2)) & 0xFFFF, 
               (schedule[i] >> (16*0)) & 0xFFFF);
    }    
    printf("\n");
}

void p_string(const char * s) {
    acquire_lock(&print_lock);
    printf(s);
    release_lock(&print_lock);
}

void p_line(const char * s) {
    acquire_lock(&print_lock);
    printf(s);
    printf("\n");
    release_lock(&print_lock);
}

void p_line_i(const char * s, volatile int i) {
    acquire_lock(&print_lock);
    printf(s);
    printf("%d\n", i);
    release_lock(&print_lock);
}

void p_line_ul(const char * s, volatile uint64_t ul) {
    acquire_lock(&print_lock);
    printf(s);
    printf("%lu\n", ul);
    release_lock(&print_lock);
}

void acquire_print_lock() {
    acquire_lock(&print_lock);
}

void release_print_lock() {
    release_lock(&print_lock);
}