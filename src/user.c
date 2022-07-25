// See LICENSE file for copyright and license details.
#include <stdint.h>

#include "config.h"
#include "kprint.h"
#include "s3k.h"

void print_cap(cap_t cap)
{
        switch (cap_get_type(cap)) {
                case CAP_TYPE_EMPTY:
                        kprintf("EMPTY {}\n");
                        break;
                case CAP_TYPE_MEMORY:
                        kprintf(
                            "MEMORY {begin=0x%lx, end=0x%lx, free=0x%lx, "
                            "rwx=%ld, pmp=%ld}\n",
                            cap_memory_get_begin(cap), cap_memory_get_end(cap), cap_memory_get_free(cap), cap_memory_get_rwx(cap), cap_memory_get_pmp(cap));
                        break;
                case CAP_TYPE_PMP:
                        kprintf("PMP {addr=0x%lx, rwx=%ld}\n", cap_pmp_get_addr(cap), cap_pmp_get_rwx(cap));
                        break;
                case CAP_TYPE_TIME:
                        kprintf(
                            "TIME {hartid=%ld, begin=%ld, end=%ld, free=%ld, "
                            "depth=%ld}\n",
                            cap_time_get_hartid(cap), cap_time_get_begin(cap), cap_time_get_end(cap), cap_time_get_free(cap), cap_time_get_depth(cap));
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
                        kprintf("SUPERVISOR {begin=%ld, end=%ld, free=%ld}\n", cap_supervisor_get_begin(cap), cap_supervisor_get_end(cap),
                                cap_supervisor_get_free(cap));
                        break;
                default:
                        kprintf("INVALID\n");
                        break;
        }
}

void user_code();

void main_supervisor(uint64_t pid, uint64_t begin, uint64_t end)
{
        kprint("Supervisor\n");

        uint64_t sup_cap = 3;
        for (int i = 1; i < N_PROC; i++) {
                cap_t new_time = cap_mk_time(1, i * 8, (i + 1) * 8, i * 8, 1);
                kprintf("Write PC %d\n", s3k_supervisor_write_reg(sup_cap, i, 0, (uint64_t)user_code));
                kprintf("Write A0 %d\n", s3k_supervisor_write_reg(sup_cap, i, 10, i));
                kprintf("Give time %d\n", s3k_derive_cap(4, 100, new_time));
                kprintf("Give cap %d\n", s3k_supervisor_give_cap(sup_cap, i, 100, 0));
                kprintf("Resume %d\n", s3k_supervisor_resume(sup_cap, i));
        }
        s3k_yield();
}

void main_uart(uint64_t pid, uint64_t begin, uint64_t end)
{
        kprint("UART\n");
}

void main_app1(uint64_t pid, uint64_t begin, uint64_t end)
{
        kprint("App1\n");
}

void main_app2(uint64_t pid, uint64_t begin, uint64_t end)
{
        kprint("App2\n");
}

void main_crypt(uint64_t pid, uint64_t begin, uint64_t end)
{
        kprint("Crypto\n");
}
