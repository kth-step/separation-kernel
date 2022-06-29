// See LICENSE file for copyright and license details.
#pragma once

#include "syscall_nr.h"
#include "types.h"

static inline uint64_t S3K_SYSCALL(uint64_t argc, uint64_t sysnr, uint64_t arg0,
                                   uint64_t arg1, uint64_t arg2, uint64_t arg3,
                                   uint64_t arg4, uint64_t arg5,
                                   uint64_t arg6) {
        register uint64_t a0 __asm__("a0") = arg0;
        register uint64_t a1 __asm__("a1") = arg1;
        register uint64_t a2 __asm__("a2") = arg2;
        register uint64_t a3 __asm__("a3") = arg3;
        register uint64_t a4 __asm__("a4") = arg4;
        register uint64_t a5 __asm__("a5") = arg5;
        register uint64_t a6 __asm__("a5") = arg6;
        register uint64_t a7 __asm__("a7") = sysnr;
        switch (argc) {
                case 1:
                        asm volatile("ecall" : "+r"(a0) : "r"(a7));
                        break;
                case 2:
                        asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a7));
                        break;
                case 3:
                        asm volatile("ecall"
                                     : "+r"(a0)
                                     : "r"(a1), "r"(a2), "r"(a7));
                        break;
                case 4:
                        asm volatile("ecall"
                                     : "+r"(a0)
                                     : "r"(a1), "r"(a2), "r"(a3), "r"(a7));
                        break;
                case 5:
                        asm volatile("ecall"
                                     : "+r"(a0)
                                     : "r"(a1), "r"(a2), "r"(a3), "r"(a4),
                                       "r"(a7));
                        break;
                case 6:
                        asm volatile("ecall"
                                     : "+r"(a0)
                                     : "r"(a1), "r"(a2), "r"(a3), "r"(a4),
                                       "r"(a5), "r"(a7));
                        break;
                case 7:
                        asm volatile("ecall"
                                     : "+r"(a0)
                                     : "r"(a1), "r"(a2), "r"(a3), "r"(a4),
                                       "r"(a5), "r"(a6), "r"(a7));
                        break;
                default:
                        __builtin_unreachable();
        }
        return a0;
}

#define S3K_SYSCALL7(a0, a1, a2, a3, a4, a5, a6) \
        S3K_SYSCALL(7, sysnr, a0, a1, a2, a3, a4, a5, a6)
#define S3K_SYSCALL6(sysnr, a0, a1, a2, a3, a4, a5) \
        S3K_SYSCALL(6, sysnr, a0, a1, a2, a3, a4, a5, 0)
#define S3K_SYSCALL5(sysnr, a0, a1, a2, a3, a4) \
        S3K_SYSCALL(5, sysnr, a0, a1, a2, a3, a4, 0, 0)
#define S3K_SYSCALL4(sysnr, a0, a1, a2, a3) \
        S3K_SYSCALL(4, sysnr, a0, a1, a2, a3, 0, 0, 0)
#define S3K_SYSCALL3(sysnr, a0, a1, a2) \
        S3K_SYSCALL(3, sysnr, a0, a1, a2, 0, 0, 0, 0)
#define S3K_SYSCALL2(sysnr, a0, a1) S3K_SYSCALL(2, sysnr, a0, a1, 0, 0, 0, 0, 0)
#define S3K_SYSCALL1(sysnr, a0) S3K_SYSCALL(1, sysnr, a0, 0, 0, 0, 0, 0, 0)

static inline uint64_t S3K_GET_PID(void) {
        return S3K_SYSCALL1(SYSNR_GET_PID, 0);
}

static inline uint64_t S3K_READ_CAP(uint64_t cid, uint64_t data[2]) {
        register int64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        register uint64_t a7 __asm__("a7");
        a0 = cid;
        a7 = 0;
        __asm__ volatile("ecall" : "+r"(a0), "=r"(a1), "=r"(a2) : "r"(a7));
        if (a0 > 0) {
                data[0] = a1;
                data[1] = a2;
        } else {
                data[0] = 0;
                data[1] = 0;
        }
        return a0 > 0;
}

static inline uint64_t S3K_MOVE_CAP(uint64_t src, uint64_t dest) {
        return S3K_SYSCALL2(SYSNR_MOVE_CAP, src, dest);
}

static inline uint64_t S3K_DELETE_CAP(uint64_t cid) {
        return S3K_SYSCALL1(SYSNR_DELETE_CAP, cid);
}

static inline uint64_t S3K_REVOKE_CAP(uint64_t cid) {
        return S3K_SYSCALL1(SYSNR_REVOKE_CAP, cid);
}
