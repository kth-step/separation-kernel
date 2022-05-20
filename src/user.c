// See LICENSE file for copyright and license details.
#include <stdio.h>

#include "lock.h"
#include "timer.h"

inline uint64_t S3K_READ_CAP(uintptr_t cid, uint64_t data[2]) {
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        register uint64_t t0 __asm__("t0");
        t0 = 1;
        a0 = cid;
        asm("ecall" : "+r"(a0), "=r"(a1), "=r"(a2) : "r"(t0));
        data[0] = a1;
        data[1] = a2;
        return a0;
}

void user_main(uintptr_t pid, uint64_t begin, uint64_t end) {
        uint64_t data[2];
        printf("pid %3lx:\t%016lx\t%016lx\r\n", pid, begin, end);
        for (int i = 0; i < 256; i++) {
                S3K_READ_CAP(i, data);
                if (S3K_READ_CAP(i, data))
                        printf("\t cid %3d:\t%016lx\t%016lx\n", i, data[0],
                               data[1]);
        }

        printf("pid %3lx:\t%016lx\t%016lx\r\n", pid, begin, end);
        for (int i = 0; i < 256; i++) {
                S3K_READ_CAP(i, data);
                if (S3K_READ_CAP(i, data))
                        printf("\t cid %3d:\t%016lx\t%016lx\n", i, data[0],
                               data[1]);
        }
        while (1)
                ;
}
