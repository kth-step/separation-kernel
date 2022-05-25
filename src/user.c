// See LICENSE file for copyright and license details.
#include <stdio.h>

#include "lock.h"
#include "s3k.h"
#include "timer.h"

void user_main(uintptr_t pid, uint64_t begin, uint64_t end) {
        uint64_t data[2];
        printf("pid %3lx:\t%016lx\t%016lx\r\n", pid, begin, end);
        for (int i = 0; i < 256; i++) {
                S3K_CAP_READ(i, data);
                if (S3K_CAP_READ(i, data))
                        printf("\t cid %3d:\t%016lx\t%016lx\n", i, data[0],
                               data[1]);
        }
        int code = S3K_TIME_SPLIT(10, 11, 4, 15, 128);
        printf("Time slice = %d\n", code);

        printf("pid %3lx:\t%016lx\t%016lx\r\n", pid, begin, end);
        for (int i = 0; i < 256; i++) {
                S3K_CAP_READ(i, data);
                if (S3K_CAP_READ(i, data))
                        printf("\t cid %3d:\t%016lx\t%016lx\n", i, data[0],
                               data[1]);
        }
        while (1)
                ;
}
