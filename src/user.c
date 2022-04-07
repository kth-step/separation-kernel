// See LICENSE file for copyright and license details.
#include <stdio.h>

#include "lock.h"
#include "timer.h"

void S3K_READ_CAP(uintptr_t cid, uint64_t *field0, uintptr_t *field1) {
        register uint64_t __field0;
        register uint64_t __field1;
        asm volatile(
            "mv a0,%2\n"
            "li t0,1\n"
            "ecall\n"
            "mv %0,a0\n"
            "mv %1,a1\n"
            : "=r&"(__field0), "=r&"(__field1)
            : "r"(cid)
            : "a0", "a1", "t0");
        *field0 = __field0;
        *field1 = __field1;
}

int S3K_MOVE_CAP(uintptr_t dest, uintptr_t src) {
        uint64_t code;
        asm volatile(
            "mv a0,%1\n"
            "mv a1,%2\n"
            "li t0,2\n"
            "ecall\n"
            "mv %0,a0\n"
            : "=r&"(code)
            : "r"(dest), "r"(src)
            : "a0", "a1", "t0");
        return code;
}

int S3K_REVOKE_CAP(uintptr_t cid) {
        uint64_t code;
        asm volatile(
            "mv a0,%1\n"
            "li t0,3\n"
            "ecall\n"
            "mv %0,a0\n"
            : "=r&"(code)
            : "r"(cid)
            : "a0", "a1", "t0");
        return code;
}

int S3K_DELETE_CAP(uintptr_t cid) {
        uint64_t code;
        asm volatile(
            "mv a0,%1\n"
            "li t0,4\n"
            "ecall\n"
            "mv %0,a0\n"
            : "=r&"(code)
            : "r"(cid)
            : "a0", "a1", "t0");
        return code;
}

int S3K_SLICE_CAP(uintptr_t src, uintptr_t dest, uintptr_t field0,
                  uintptr_t field1) {
        uint64_t code;
        asm volatile(
            "mv a0,%1\n"
            "mv a1,%2\n"
            "mv a2,%3\n"
            "mv a3,%4\n"
            "li t0,5\n"
            "ecall\n"
            "mv %0,a0\n"
            : "=r&"(code)
            : "r"(src), "r"(dest), "r"(field0), "r"(field1)
            : "a0", "a1", "a2", "a3", "t0");
        return code;
}

void user_main(uintptr_t pid, uint64_t begin, uint64_t end) {
        while (1) {
                printf("pid %3lx:\t%016lx\t%016lx\r\n", pid, begin, end);
                int code;
                code = S3K_SLICE_CAP(2, 20, 0x0300011000000040,
                                     0x0000000000000000);
                printf("%d\n", code);
                S3K_SLICE_CAP(20, 9, 0x0300020400000040, 0x0000000000000000);
                S3K_DELETE_CAP(20);
                for (int i = 0; i < 256; i++) {
                        uint64_t field0, field1;
                        S3K_READ_CAP(i, &field0, &field1);
                        if (field0 != 0 || field1 != 0)
                                printf("\t cid %3d:\t%016lx\t%016lx\n", i,
                                       field0, field1);
                }

                for (int i = 0; i < 256; i++) {
                        if (!S3K_REVOKE_CAP(i)) {
                                printf("cid %3d revoke success\n", i);
                                for (int i = 0; i < 256; i++) {
                                        uint64_t field0, field1;
                                        S3K_READ_CAP(i, &field0, &field1);
                                        if (field0 != 0 || field1 != 0)
                                                printf(
                                                    "\t cid "
                                                    "%3d:\t%016lx\t%016lx\n",
                                                    i, field0, field1);
                                }
                        }
                }
        }
}
