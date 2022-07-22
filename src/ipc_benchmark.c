// See LICENSE file for copyright and license details.

#include <stdint.h>
#include <stdio.h>

#include "config.h"
#include "s3k.h"


#include <stdlib.h>
// TODO: probably remove all prints for failure when we know it works, since we currently don't deschedule after first success
// resulting in many iterations of failure before rescheduling.

volatile uint64_t ipc_counter = 0;

void ipc_benchmark_main(uint64_t pid) {
    while (1) {
        //printf("Pid: %lu\n", pid); ////// Temp
        if (pid == N_PROC - 1) {
            ipc_counter++;
            printf("\nCounter: %lu\n", ipc_counter);
            exit(0); ////// Temp
            // Delete owned time slice and await new message. 
            // Should be fine to delete own time slice before knowing that the next process has been able to utilzie the child.
        } 
        else if (pid == 0) {
            // Caps for boot: pmp, memory, channels, time for each hart, supervisor for each proc, sender.
            // First empty capslot in cap_table is thus: 3 + N_CORES + N_PROC + 1
            int new_cap_ind = 3 + N_CORES + N_PROC + 1;
            if (S3K_SLICE_TIME(3, new_cap_ind, 0, N_QUANTUM, 1, 255) < 1) {
                printf("Failed slice time. Pid: %lu\n", pid);
            }
            if (S3K_SEND(new_cap_ind - 1, new_cap_ind, 1, (uint64_t[4]){0,0,0,0}) < 1) {
                printf("Failed send. Pid: %lu\n", pid);
            }
            // Delete owned time slice and await new message. 
            // Should be fine to delete own time slice before knowing that the next process has been able to utilzie the child.
        } 
        else {
            if (S3K_SLICE_TIME(2, 3, 0, N_QUANTUM, pid + 1, 255) < 1) {
                printf("Failed slice time. Pid: %lu\n", pid);
            }
            if (S3K_SEND(1, 3, 1, (uint64_t[4]){0,0,0,0}) < 1) {
                printf("Failed send. Pid: %lu\n", pid);
            }
        }
    }
}