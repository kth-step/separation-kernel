// See LICENSE file for copyright and license details.
#include <stdio.h>

#include "config.h"
#include "lock.h"
#include "s3k.h"
#include "timer.h"

void user_main(uintptr_t pid, uint64_t begin, uint64_t end) {
        while (1) {
                uint64_t data[2];
                printf("pid %3lx:\t%016lx\t%016lx\r\n", pid, begin, end);
                for (int i = 0; i < 256; i++) {
                        if (S3K_READ_CAP(i, data))
                                printf("\t cid %3d:\t%016lx\t%016lx\n", i,
                                       data[0], data[1]);
                }
                int code;
                uint64_t addr = USER_MEMORY_BEGIN + ((0x2000 - 1) >> 1);

                code = S3K_MAKE_PMP(1,25, addr, 3);
                printf("Make PMP = %d\n", code);

                code = S3K_SPLIT_TIME(4, 10, 11, 250, 128);
                printf("Split time = %d\n", code);

                printf("pid %3lx:\t%016lx\t%016lx\r\n", pid, begin, end);
                for (int i = 0; i < 256; i++) {
                        if (S3K_READ_CAP(i, data))
                                printf("\t cid %3d:\t%016lx\t%016lx\n", i,
                                       data[0], data[1]);
                }
                for (int i = 1; i < 256; i++) {
                        S3K_REVOKE_CAP(i);
                }
        }
}
