// See LICENSE file for copyright and license details.
#include <string.h>

#include "config.h"
#include "kprint.h"
#include "s3k.h"
#include "types.h"

void print_cap(Cap cap) {
        switch (cap_get_type(cap)) {
                case CAP_MEMORY:
                        kprintf("MEMORY SLICE");
                        kprintf(" begin=0x%lx", cap_memory_get_begin(cap));
                        kprintf(" free=0x%lx", cap_memory_get_free(cap));
                        kprintf(" end=0x%lx", cap_memory_get_end(cap));
                        kprintf(" rwx=%ld", cap_memory_get_rwx(cap));
                        kprintf(" pmp=%ld", cap_memory_get_pmp(cap));
                        break;
                case CAP_PMP:
                        kprintf("PMP FRAME   ");
                        kprintf(" addr=0x%lx", cap_pmp_get_addr(cap));
                        kprintf(" rwx=%ld", cap_pmp_get_rwx(cap));
                        break;
                case CAP_TIME:
                        kprintf("TIME SLICE  ");
                        kprintf(" hartid=%ld", cap_time_get_hartid(cap));
                        kprintf(" pid=%ld", cap_time_get_pid(cap));
                        kprintf(" begin=%ld", cap_time_get_begin(cap));
                        kprintf(" free=%ld", cap_time_get_free(cap));
                        kprintf(" end=%ld", cap_time_get_end(cap));
                        kprintf(" depth=%ld", cap_time_get_depth(cap));
                        break;
                case CAP_CHANNELS:
                        kprintf("CHANNELS    ");
                        kprintf(" begin=%ld", cap_channels_get_begin(cap));
                        kprintf(" free=%ld", cap_channels_get_free(cap));
                        kprintf(" end=%ld", cap_channels_get_end(cap));
                        break;
                case CAP_ENDPOINT:
                        kprintf("ENDPOINT    ");
                        kprintf(" channel=%ld", cap_endpoint_get_channel(cap));
                        kprintf(" mode=%ld", cap_endpoint_get_mode(cap));
                        break;
                case CAP_SUPERVISOR:
                        kprintf("SUPERVISOR  ");
                        kprintf(" begin=%ld", cap_supervisor_get_begin(cap));
                        kprintf(" free=%ld", cap_supervisor_get_free(cap));
                        kprintf(" end=%ld", cap_supervisor_get_end(cap));
                        break;
                default:
                        kprintf("INVALID");
                        break;
        }
        kprintf("\n");
}
void user_code();
void user_main(uintptr_t pid, uint64_t begin, uint64_t end) {
        Cap cap;
        kprintf("\n\n");
        kprintf("pid %3lx:\t%016lx\t%016lx\r\n", pid, begin, end);
        kprintf("\n");
        kprintf("Printing available capabilities\n");
        kprintf("--------------------------------\n");
        for (int i = 0; i < 30; i++) {
                cap = s3k_read_cap(i);
                if (cap_get_type(cap) != CAP_INVALID) {
                        kprintf("%d: ", i);
                        print_cap(cap);
                }
        }

        if (pid == 0) {
                Cap new_time = cap_mk_time(2, 1, 10, 32, 10, 2);
                uint64_t sup_cap = 3;
                uint64_t supervisee_pid = 1;
                uint64_t time_cap = 8;
                kassert(s3k_derive_cap(5, time_cap, new_time));
                kassert(s3k_supervisor_give_cap(sup_cap, supervisee_pid, time_cap,
                                             1, 1));
                kassert(s3k_supervisor_write_reg(sup_cap, supervisee_pid, 0,
                                         (uint64_t)user_code));
                kassert(s3k_supervisor_resume(sup_cap, supervisee_pid));
        }

}
