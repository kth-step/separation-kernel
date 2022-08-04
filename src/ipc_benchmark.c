// See LICENSE file for copyright and license details.

#include <stdint.h>
#include <stdio.h>

#include "config.h"
#include "s3k.h"
#include "timer.h"

#define IPC_DEBUG 0

#if IPC_DEBUG != 0
    volatile uint64_t ipc_counter1 = 0;
    volatile uint64_t ipc_counter2 = 0;
#endif
volatile int round_counter = 0;
uint64_t values[N_CORES][BENCHMARK_ROUNDS];

void ipc_benchmark_main(uint64_t pid) {
    #if TIME_SLOT_LOANING == 0 && TIME_SLOT_LOANING_SIMPLE == 0
    printf("\n");
    while (1) {
        #if IPC_DEBUG != 0
            printf("Pid: %lu\n", pid);
        #endif
        if (pid == 0) {
            uint64_t start_time = read_time();
            /* Caps for boot: pmp, memory, channels, time for each hart, supervisor for each proc, sender.
               First empty capslot in cap_table is thus: 3 + N_CORES + N_PROC + 1 */
            int new_cap_ind = 3 + N_CORES + N_PROC + 1;
            if (S3K_SLICE_TIME(3, new_cap_ind, 0, N_QUANTUM, 1, 255) < 1) {
                printf("Failed slice time, pid: %lu\n", pid);
                continue;
            }
            if (S3K_SEND(new_cap_ind - 1, new_cap_ind, 1, (uint64_t[4]){0,0,0,0}) < 1) {
                printf("Failed send, pid: %lu\n", pid);
                continue;
            }
            if (S3K_YIELD() < 1) {
                printf("Failed yield, pid: %lu\n", pid);
                continue;
            }
            uint64_t end_time = read_time();
            //printf("\nValue=%lu\n", end_time-start_time);
            //round_counter++;
            // TODO: if we want to support multicore testing this needs changing
            values[0][round_counter++] = end_time-start_time;
        
            #if IPC_DEBUG != 0
                ipc_counter2++;
                printf("Counter on first proc: %lu\n", ipc_counter2);
            #endif
        }
        #if ONLY_2_PROC_IPC != 0
        else if (pid == 1) {
        #else 
        else if (pid == N_PROC - 1) {
        #endif
            #if IPC_DEBUG != 0
                ipc_counter1++;
                printf("Counter on last proc:  %lu\n", ipc_counter1);
            #endif
            uint64_t msg[4];
            /* Delete owned time slice and await new message. */
            if (S3K_RECV_DELETE_TS(1, 3, 1, msg, 3) < 1) {
                printf("Failed recv-delete, pid: %lu\n", pid);
                continue;
            }
        }
        else {
            if (S3K_SLICE_TIME(3, 4, 0, N_QUANTUM, pid + 1, 255) < 1) {
                printf("Failed slice time, pid: %lu\n", pid);
                continue;
            }
            if (S3K_SEND(2, 4, 1, (uint64_t[4]){0,0,0,0}) < 1) {
                printf("Failed send, pid: %lu\n", pid);
                continue;
            }
            /* We yield because if we try to delete now we fail because the 
               delete sched_update won't be able to update anything;
               this because all time slots that would be updated belong
               to the receiver of our send request instead. */
               // TODO: should delete sched_update return false in this case, and should sn_recv_delete_ts fail from this?
            if (S3K_YIELD() < 1) {
                printf("Failed yield, pid: %lu\n", pid);
                continue;
            }
            uint64_t msg[4];
            /* Delete owned time slice and await new message. */
            if (S3K_RECV_DELETE_TS(1, 3, 1, msg, 3) < 1) {
                printf("Failed recv-delete, pid: %lu\n", pid);
                continue;
            }
        }
    }
    #else
    // This will probably not work for TIME_SLOT_LOANING, but rather only for TIME_SLOT_LOANING_SIMPLE
    printf("\n");
    while (1) {
        #if IPC_DEBUG != 0
            printf("Pid: %lu\n", pid);
        #endif
        if (pid == 0) {
            uint64_t start_time = read_time();
            /* Caps for boot: pmp, memory, channels, time for each hart, supervisor for each proc, sender.
               First empty capslot in cap_table is thus: 3 + N_CORES + N_PROC + 1 */
            if (S3K_LOAN_TIME(pid+1) < 1) {
                printf("Failed loan time, pid: %lu\n", pid);
                continue;
            }
            uint64_t end_time = read_time();
            //printf("\nValue=%lu\n", end_time-start_time);
            //round_counter++;
            // TODO: if we want to support multicore testing this needs changing
            values[0][round_counter++] = end_time-start_time;
        
            #if IPC_DEBUG != 0
                ipc_counter2++;
                printf("Counter on first proc: %lu\n", ipc_counter2);
            #endif
        } 
        #if ONLY_2_PROC_IPC != 0
        else if (pid == 1) {
        #else 
        else if (pid == N_PROC - 1) {
        #endif
            #if IPC_DEBUG != 0
                ipc_counter1++;
                printf("Counter on last proc:  %lu\n", ipc_counter1);
            #endif
            if (S3K_RETURN_LOANED_TIME() < 1) {
                printf("Failed return time, pid: %lu\n", pid);
                continue;
            }
        }
        else {
            if (S3K_LOAN_TIME(pid+1) < 1) {
                printf("Failed loan time, pid: %lu\n", pid);
                continue;
            }
            if (S3K_RETURN_LOANED_TIME() < 1) {
                printf("Failed return time, pid: %lu\n", pid);
                continue;
            }
        }
    }
    #endif
}