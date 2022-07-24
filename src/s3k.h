// See LICENSE file for copyright and license details.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "s3k_cap.h"
#include "s3k_consts.h"

static inline uint64_t S3K_SYSCALL(uint64_t argc, uint64_t sysnr, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5,
                                   uint64_t arg6)
{
        register uint64_t a0 __asm__("a0") = arg0;
        register uint64_t a1 __asm__("a1") = arg1;
        register uint64_t a2 __asm__("a2") = arg2;
        register uint64_t a3 __asm__("a3") = arg3;
        register uint64_t a4 __asm__("a4") = arg4;
        register uint64_t a5 __asm__("a5") = arg5;
        register uint64_t a6 __asm__("a6") = arg6;
        register uint64_t a7 __asm__("a7") = sysnr;
        switch (argc) {
                case 0:
                        asm volatile("ecall" : "=r"(a0) : "r"(a7));
                        break;
                case 1:
                        asm volatile("ecall" : "+r"(a0) : "r"(a7));
                        break;
                case 2:
                        asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a7));
                        break;
                case 3:
                        asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7));
                        break;
                case 4:
                        asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a7));
                        break;
                case 5:
                        asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a7));
                        break;
                case 6:
                        asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a7));
                        break;
                case 7:
                        asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7));
                        break;
                case 8:
                        asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7), "r"(a7));
                        break;
                default:
                        while (1)
                                ;
        }
        return a0;
}

#define S3K_SYSCALL8(a0, a1, a2, a3, a4, a5, a6, a7) S3K_SYSCALL(8, sysnr, a0, a1, a2, a3, a4, a5, a6)
#define S3K_SYSCALL7(a0, a1, a2, a3, a4, a5, a6) S3K_SYSCALL(7, sysnr, a0, a1, a2, a3, a4, a5, a6)
#define S3K_SYSCALL6(sysnr, a0, a1, a2, a3, a4, a5) S3K_SYSCALL(6, sysnr, a0, a1, a2, a3, a4, a5, 0)
#define S3K_SYSCALL5(sysnr, a0, a1, a2, a3, a4) S3K_SYSCALL(5, sysnr, a0, a1, a2, a3, a4, 0, 0)
#define S3K_SYSCALL4(sysnr, a0, a1, a2, a3) S3K_SYSCALL(4, sysnr, a0, a1, a2, a3, 0, 0, 0)
#define S3K_SYSCALL3(sysnr, a0, a1, a2) S3K_SYSCALL(3, sysnr, a0, a1, a2, 0, 0, 0, 0)
#define S3K_SYSCALL2(sysnr, a0, a1) S3K_SYSCALL(2, sysnr, a0, a1, 0, 0, 0, 0, 0)
#define S3K_SYSCALL1(sysnr, a0) S3K_SYSCALL(1, sysnr, a0, 0, 0, 0, 0, 0, 0)
#define S3K_SYSCALL0(sysnr) S3K_SYSCALL(0, sysnr, 0, 0, 0, 0, 0, 0, 0)

static inline uint64_t s3k_get_pid(void)
{
        return S3K_SYSCALL1(S3K_SYSNR_GET_PID, 0);
}

static inline uint64_t s3k_yield()
{
        return S3K_SYSCALL1(S3K_SYSNR_YIELD, 0);
}

static inline cap_t s3k_read_cap(uint64_t cid)
{
        register int64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        register uint64_t a7 __asm__("a7");
        a0 = cid;
        a7 = S3K_SYSNR_READ_CAP;
        __asm__ volatile("ecall" : "+r"(a0), "=r"(a1), "=r"(a2) : "r"(a7));
        if (a0 == S3K_EMPTY)
                return (cap_t){0, 0};
        return (cap_t){a1, a2};
}

static inline uint64_t s3k_move_cap(uint64_t src, uint64_t dest)
{
        return S3K_SYSCALL2(S3K_SYSNR_MOVE_CAP, src, dest);
}

static inline uint64_t s3k_delete_cap(uint64_t cid)
{
        return S3K_SYSCALL1(S3K_SYSNR_DELETE_CAP, cid);
}

static inline uint64_t s3k_revoke_cap(uint64_t cid)
{
        return S3K_SYSCALL1(S3K_SYSNR_REVOKE_CAP, cid);
}

static inline uint64_t s3k_derive_cap(uint64_t src, uint64_t dest, cap_t cap)
{
        return S3K_SYSCALL4(S3K_SYSNR_DERIVE_CAP, src, dest, cap.word0, cap.word1);
}

static inline uint64_t s3k_supervisor_suspend(uint64_t sup_cid, uint64_t pid)
{
        return S3K_SYSCALL2(S3K_SYSNR_SUPERVISOR_SUSPEND, sup_cid, pid);
}

static inline uint64_t s3k_supervisor_resume(uint64_t sup_cid, uint64_t pid)
{
        return S3K_SYSCALL2(S3K_SYSNR_SUPERVISOR_RESUME, sup_cid, pid);
}

static inline uint64_t s3k_supervisor_read_reg(uint64_t sup_cid, uint64_t pid, uint64_t reg_nr)
{
        return S3K_SYSCALL3(S3K_SYSNR_SUPERVISOR_READ_REG, sup_cid, pid, reg_nr);
}

static inline uint64_t s3k_supervisor_write_reg(uint64_t sup_cid, uint64_t pid, uint64_t reg_nr, uint64_t val)
{
        return S3K_SYSCALL4(S3K_SYSNR_SUPERVISOR_WRITE_REG, sup_cid, pid, reg_nr, val);
}

static inline uint64_t s3k_supervisor_read_caps(uint64_t sup_cid, uint64_t pid, uint64_t cid)
{
        return S3K_SYSCALL3(S3K_SYSNR_SUPERVISOR_READ_CAP, sup_cid, pid, cid);
}

static inline uint64_t s3k_supervisor_give_caps(uint64_t sup_cid, uint64_t pid, uint64_t src, uint64_t dest)
{
        return S3K_SYSCALL4(S3K_SYSNR_SUPERVISOR_GIVE_CAP, sup_cid, pid, src, dest);
}

static inline uint64_t s3k_supervisor_take_caps(uint64_t sup_cid, uint64_t pid, uint64_t src, uint64_t dest)
{
        return S3K_SYSCALL4(S3K_SYSNR_SUPERVISOR_TAKE_CAP, sup_cid, pid, src, dest);
}

static inline void s3k_dump_cap(char* buf, int n, cap_t cap)
{ /* enum cap_type type = cap_get_type(cap); */
}
