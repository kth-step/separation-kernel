// See LICENSE file for copyright and license details.
#include <stdint.h>

#include "config.h"
#include "kprint.h"
#include "lock.h"
#include "s3k.h"

extern void user_code();

void dump_cap(char* name)
{
        static lock_t lock = INIT_LOCK;
        lock_acquire(&lock);

        kprintf("\n%s capabilities\n", name);
        for (int i = 0; i < 30; i++) {
                cap_t cap = s3k_read_cap(i);
                char buf[256];
                if (cap_get_type(cap) != CAP_TYPE_EMPTY) {
                        s3k_dump_cap(buf, 256, cap);
                        kprintf("%d: %s\n", i, buf);
                }
        }
        lock_release(&lock);
}

void main_supervisor(uint64_t pid, uint64_t begin, uint64_t end)
{
        dump_cap("supervisor");
        // s3k_delete_cap(5);
        s3k_delete_cap(6);
        s3k_delete_cap(7);
        cap_t new_receiver = cap_mk_receiver(0);
        cap_t new_sender = cap_mk_sender(0, 1);
        s3k_derive_cap(2, 20, new_receiver);
        s3k_derive_cap(20, 21, new_sender);
        dump_cap("supervisor");
        s3k_supervisor_give_cap(3, 1, 4, 0);
        s3k_supervisor_give_cap(3, 1, 21, 1);
        s3k_supervisor_write_reg(3, 1, 0, (uint64_t)(user_code));
        s3k_supervisor_write_reg(3, 1, 10, 1);
        kprintf("Supervisor resume %d\n", s3k_supervisor_resume(3, 1));
        uint64_t msg[4];
        kprintf("Receive message %d\n", s3k_receive(20, msg, 10, 0));
        kprintf("Message %d,%d,%d,%d\n", msg[0], msg[1], msg[2], msg[3]);
        s3k_yield();
        dump_cap("supervisor");
}

void main_uart_in(uint64_t pid, uint64_t begin, uint64_t end)
{
        dump_cap("uart_in");
}

void main_uart_out(uint64_t pid, uint64_t begin, uint64_t end)
{
        dump_cap("uart_out");
}

void main_app1(uint64_t pid, uint64_t begin, uint64_t end)
{
        dump_cap("app1");
}

void main_app2(uint64_t pid, uint64_t begin, uint64_t end)
{
        dump_cap("app2");
}

void main_crypt(uint64_t pid, uint64_t begin, uint64_t end)
{
        dump_cap("crypto");
}
