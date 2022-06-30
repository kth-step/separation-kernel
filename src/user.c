// See LICENSE file for copyright and license details.
#include "config.h"
#include "types.h"
#include "s3k.h"
#include "kprint.h"

void user_main(uintptr_t pid, uint64_t begin, uint64_t end) {
loop:
        uint64_t data[2];
        kprintf("TESTING\n");
        kprintf("pid %3lx:\t%016lx\t%016lx\r\n", pid, begin, end);
        for (int i = 0; i < 30; i++) {
                if (S3K_READ_CAP(i, data))
                        kprintf("\t cid %3d:\t%016lx\t%016lx\n", i, data[0],
                               data[1]);
        }
        //   int code;
        //   uint64_t addr = USER_MEMORY_BEGIN + ((0x2000 - 1) >> 1);

        //   code = S3K_MAKE_PMP(1, 25, addr, 3);
        //   kprintf("Make PMP from 1 to 25 = %d\n", code);

        //   code = S3K_SPLIT_TIME(4, 10, 11, 250, 128);
        //   kprintf("Split time 4 to 10, 11 = %d\n", code);
        //   code = S3K_DELETE_CAP(4);
        //   kprintf("Delete cap 4 = %d\n", code);

        kprintf("pid %3lx:\t%016lx\t%016lx\r\n", pid, begin, end);
        for (int i = 0; i < 30; i++) {
                if (S3K_READ_CAP(i, data))
                        kprintf("\t cid %3d:\t%016lx\t%016lx\n", i, data[0],
                               data[1]);
        }
        kprintf("Revoking caps\n");
        for (int i = 1; i < 30; i++) {
                S3K_REVOKE_CAP(i);
        }
        for (int i = 0; i < 30; i++) {
                if (S3K_READ_CAP(i, data))
                        kprintf("\t cid %3d:\t%016lx\t%016lx\n", i, data[0],
                               data[1]);
        }
      goto loop;
}
