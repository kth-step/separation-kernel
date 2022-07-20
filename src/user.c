// See LICENSE file for copyright and license details.
#include <stdint.h>
#include <string.h>

#include "config.h"
#include "kprint.h"
#include "s3k.h"

void print_cap(cap_t cap)
{
    switch (cap_get_type(cap)) {
    case CAP_TYPE_MEMORY:
        kprintf("MEMORY {begin=0x%lx, end=0x%lx, free=0x%lx, rwx=%ld, pmp=%ld}\n", cap_memory_get_begin(cap),
            cap_memory_get_end(cap), cap_memory_get_free(cap), cap_memory_get_rwx(cap), cap_memory_get_pmp(cap));
        break;
    case CAP_TYPE_PMP:
        kprintf("PMP {addr=0x%lx, rwx=%ld}\n", cap_pmp_get_addr(cap), cap_pmp_get_rwx(cap));
        break;
    case CAP_TYPE_TIME:
        kprintf("TIME {hartid=%ld, begin=%ld, end=%ld, free=%ld, depth=%ld}\n", cap_time_get_hartid(cap),
            cap_time_get_begin(cap), cap_time_get_end(cap), cap_time_get_free(cap), cap_time_get_depth(cap));
        break;
    case CAP_TYPE_CHANNELS:
        kprintf("CHANNELS {begin=%ld, end=%ld, free=%ld}\n", cap_channels_get_begin(cap), cap_channels_get_free(cap),
            cap_channels_get_end(cap));
        break;
    case CAP_TYPE_RECEIVER:
        kprintf("RECEIVER {channel=%ld}\n", cap_receiver_get_channel(cap));
        break;
    case CAP_TYPE_SENDER:
        kprintf("SENDER {channel=%ld}\n", cap_sender_get_channel(cap));
        break;
    case CAP_TYPE_SUPERVISOR:
        kprintf("SUPERVISOR {begin=%ld, end=%ld, free=%ld}\n", cap_supervisor_get_begin(cap),
            cap_supervisor_get_free(cap), cap_supervisor_get_end(cap));
        break;
    default:
        kprintf("INVALID\n");
        break;
    }
}

void user_code();
void user_main(uint64_t pid, uint64_t begin, uint64_t end)
{
    cap_t cap;
    uint64_t code;
    kprintf("\n\n");
    kprintf("pid %3lx:\t%016lx\t%016lx\r\n", pid, begin, end);
    kprintf("\n");
    kprintf("Printing available capabilities\n");
    kprintf("--------------------------------\n");
    for (int i = 0; i < 30; i++) {
        cap = s3k_read_cap(i);
        if (cap_get_type(cap) != CAP_TYPE_EMPTY) {
            kprintf("%3d: ", i);
            print_cap(cap);
            s3k_yield();
        }
    }

    if (pid == 0) {
        cap_t new_time = cap_mk_time(2, 10, 32, 10, 1);
        cap_t new_recvpoint = cap_mk_receiver(5);
        cap_t new_sendpoint = cap_mk_sender(5, 1);
        uint64_t sup_cap = 4;
        uint64_t supervisee_pid = 1;
        kprintf("Derive time, %d\n", s3k_derive_cap(6, 9, new_time));
        s3k_yield();
        for (int i = 0; i < 6; i++) {
            cap_t new_time = cap_mk_time(1, i * 8, (i + 1) * 8, i * 8, 1);
            s3k_derive_cap(5, 24 + i, new_time);
            s3k_yield();
        }
        kprintf("Move time cap %d\n", s3k_move_cap(9, 10));
        s3k_yield();
        kprintf("Derive receiver %d\n", s3k_derive_cap(3, 11, new_recvpoint));
        s3k_yield();
        kprintf("Derive sender %d\n", s3k_derive_cap(11, 12, new_sendpoint));
        s3k_yield();

        for (int i = 0; i < 30; i++) {
            cap = s3k_read_cap(i);
            if (cap_get_type(cap) != CAP_TYPE_EMPTY) {
                kprintf("%3d: ", i);
                print_cap(cap);
                s3k_yield();
            }
        }

        kprintf("Give cap %d\n", s3k_supervisor_give_caps(sup_cap, supervisee_pid, 10, 1));
        s3k_yield();
        kprintf("Give cap %d\n", s3k_supervisor_give_caps(sup_cap, supervisee_pid, 12, 2));
        s3k_yield();
        kprintf("Write reg %d\n", s3k_supervisor_write_reg(sup_cap, supervisee_pid, 0, (uint64_t)user_code));
        s3k_yield();
        kprintf("Resume %d\n", s3k_supervisor_resume(sup_cap, supervisee_pid));
        s3k_yield();
        kprintf("Receive %d\n", (code = S3K_SYSCALL3(S3K_SYSNR_IPC_RECEIVE, 11, 20, 8)));
        s3k_yield();
        int i = 0;
        while (1) {
            s3k_get_pid();
            s3k_yield();
            i++;
        }

        if (code != 0)
            return;

        kprintf("Dumping capabilities\n");
        for (int i = 0; i < 30; i++) {
            cap = s3k_read_cap(i);
            if (cap_get_type(cap) != CAP_TYPE_EMPTY) {
                kprintf("%3d: ", i);
                print_cap(cap);
                s3k_yield();
            }
        }
    } else if (pid == 1) {
        kprintf("Send %d\n", (code = S3K_SYSCALL3(S3K_SYSNR_IPC_SEND, 2, 20, 8)));
        s3k_yield();
    }
}
