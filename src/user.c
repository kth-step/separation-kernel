// See LICENSE file for copyright and license details.
#include <string.h>

#include <stdint.h>
#include "config.h"
#include "kprint.h"
#include "s3k.h"

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

int wait(uint64_t t) {
   uint64_t time = read_time();
   while (read_time() - t < time) {

   }
   return read_time();
}

void dump_capabilities() {
        Cap cap;
        kprintf("Printing available capabilities\n");
        kprintf("--------------------------------\n");
        for (int i = 0; i < 30; i++) {
                cap = s3k_read_cap(i);
                if (cap_get_type(cap) != CAP_INVALID) {
                        kprintf("%d: ", i);
                        print_cap(cap);
                }
        }

}

#define WAIT_TIME 100000

void boot_init() {
        kprintf("boot init start: %d\n", read_time());
        s3k_delete_cap(5);
        s3k_delete_cap(6);
        s3k_delete_cap(7);

        Cap new_time = cap_mk_time(1, 1, 10, 32, 10, 2);
        uint64_t sup_cap = 3;
        uint64_t supervisee_pid = 1;
        uint64_t time_cap = 8;
        kprintf("%d\n", s3k_derive_cap(4, time_cap, new_time));
        dump_capabilities();

        kprintf("%d\n", s3k_supervisor_give_cap(sup_cap, supervisee_pid, time_cap, 1, 1));
        dump_capabilities();

        kprintf("%d\n", s3k_supervisor_write_reg(sup_cap, supervisee_pid, 0, (uint64_t)user_code));
        kprintf("%d\n", s3k_supervisor_resume(sup_cap, supervisee_pid));

        new_time = cap_mk_time(1, 2, 33, 40, 33, 2);
        supervisee_pid = 2;
        time_cap = 8;
        dump_capabilities();
        kprintf("derive %d\n", s3k_derive_cap(4, time_cap, new_time));
        dump_capabilities();
        kprintf("give %d\n", s3k_supervisor_give_cap(sup_cap, supervisee_pid, time_cap, 1, 1));
        dump_capabilities();
        kprintf("%d\n", s3k_supervisor_write_reg(sup_cap, supervisee_pid, 0, (uint64_t)user_code));
        kprintf("%d\n", s3k_supervisor_resume(sup_cap, supervisee_pid));

        kprintf("boot init end: %d\n", read_time());

        int i = 0;
        while (1) {
                wait(WAIT_TIME);
                kprintf("boot init loop: %d %d\n", i++, read_time());
        }
}

void uart_driver_init() {
        kprintf("Starting UART driver\n");
        while (1) {
                wait(WAIT_TIME);
                kprintf("driver loop: %d\n", read_time());
        }
}

void client_init () {
        kprintf("Starting client\n");
        while (1) {
                wait(WAIT_TIME);
                kprintf("client loop: %d\n", read_time());
        }
}
void user_main(uintptr_t pid, uint64_t begin, uint64_t end) {
        kprintf("\n\n");
        kprintf("pid %3lx:\t%016lx\t%016lx\r\n", pid, begin, end);
        kprintf("\n");
        dump_capabilities();

        if (pid == 0) {
                boot_init();
        } else if (pid == 1) {
                uart_driver_init();
        } else if (pid == 2) {
                client_init();
        }
}
