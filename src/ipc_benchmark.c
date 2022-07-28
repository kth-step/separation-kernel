// See LICENSE file for copyright and license details.

#include <stdint.h>
#include <stdio.h>

#include "config.h"
#include "s3k.h"

volatile uint64_t ipc_counter = 0;

void ipc_benchmark_main(uint64_t pid) {
    printf("\n");
    while (1) {
        printf("Pid: %lu\n", pid); ////// Temp
          if (pid == 0) {
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
        } else if (pid == N_PROC - 1) {
            ipc_counter++;
            printf("\nCounter: %lu\n", ipc_counter);
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
}