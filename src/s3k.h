// See LICENSE file for copyright and license details.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "s3k_cap.h"
#include "s3k_consts.h"
#include "snprintf.h"

static inline uint64_t S3K_SYSCALL(uint64_t argc, uint64_t sysnr, uint64_t arg0, uint64_t arg1, uint64_t arg2,
                                   uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6)
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
                asm volatile("ecall"
                             : "+r"(a0)
                             : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7), "r"(a7));
                break;
        default:
                while (1)
                        ;
        }
        return a0;
}

#define S3K_SYSCALL7(sysnr, a0, a1, a2, a3, a4, a5, a6) S3K_SYSCALL(7, sysnr, a0, a1, a2, a3, a4, a5, a6)
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
        register uint64_t a7 __asm__("a7");
        a0 = cid;
        a7 = S3K_SYSNR_READ_CAP;
        __asm__ volatile("ecall" : "+r"(a0), "=r"(a1) : "r"(a7));
        return (cap_t){a0, a1};
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

static inline uint64_t s3k_receive(uint64_t cid, uint64_t msg[4], uint64_t dest, uint64_t flags)
{
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        register uint64_t a3 __asm__("a3");
        register uint64_t a4 __asm__("a4");
        register uint64_t a5 __asm__("a5");
        register uint64_t a6 __asm__("a6");
        register uint64_t a7 __asm__("a7");
        a0 = cid;
        a5 = dest;
        a6 = flags;
        a7 = S3K_SYSNR_INVOKE_CAP;
        __asm__("ecall" : "+r"(a0), "=r"(a1), "=r"(a2), "=r"(a3), "=r"(a4) : "r"(a5), "r"(a6), "r"(a7));
        msg[0] = a1;
        msg[1] = a2;
        msg[2] = a3;
        msg[3] = a4;
        return a0;
}

static inline uint64_t s3k_send(uint64_t cid, uint64_t msg[4], uint64_t src, uint64_t flags)
{
        return S3K_SYSCALL7(S3K_SYSNR_INVOKE_CAP, cid, msg[0], msg[1], msg[2], msg[3], src, flags);
}

static inline uint64_t s3k_supervisor_suspend(uint64_t sup_cid, uint64_t pid)
{
        return S3K_SYSCALL3(S3K_SYSNR_INVOKE_CAP, sup_cid, pid, 0);
}

static inline uint64_t s3k_supervisor_resume(uint64_t sup_cid, uint64_t pid)
{
        return S3K_SYSCALL3(S3K_SYSNR_INVOKE_CAP, sup_cid, pid, 1);
}

static inline uint64_t s3k_supervisor_get_state(uint64_t sup_cid, uint64_t pid)
{
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        register uint64_t a7 __asm__("a7");
        a0 = sup_cid;
        a1 = pid;
        a2 = 5;
        a7 = S3K_SYSNR_INVOKE_CAP;
        __asm__ volatile("ecall" : "+r"(a0), "+r"(a1) : "r"(a2), "r"(a7));
        if (a0 != S3K_OK)
                return -1;
        return a1;
}

static inline uint64_t s3k_supervisor_read_reg(uint64_t sup_cid, uint64_t pid, uint64_t reg_nr)
{
        return S3K_SYSCALL4(S3K_SYSNR_INVOKE_CAP, sup_cid, pid, 3, reg_nr);
}

static inline uint64_t s3k_supervisor_write_reg(uint64_t sup_cid, uint64_t pid, uint64_t reg_nr, uint64_t val)
{
        return S3K_SYSCALL5(S3K_SYSNR_INVOKE_CAP, sup_cid, pid, 4, reg_nr, val);
}

static inline cap_t s3k_supervisor_read_cap(uint64_t sup_cid, uint64_t pid, uint64_t cid)
{
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        register uint64_t a3 __asm__("a3");
        register uint64_t a7 __asm__("a7");
        a0 = sup_cid;
        a1 = pid;
        a2 = 5;
        a3 = cid;
        a7 = S3K_SYSNR_INVOKE_CAP;
        __asm__ volatile("ecall" : "+r"(a0), "+r"(a1), "+r"(a2) : "r"(a3), "r"(a7));
        if (a0 == S3K_OK)
                return (cap_t){a1, a2};
        return NULL_CAP;
}

static inline uint64_t s3k_supervisor_give_cap(uint64_t sup_cid, uint64_t pid, uint64_t src, uint64_t dest)
{
        return S3K_SYSCALL5(S3K_SYSNR_INVOKE_CAP, sup_cid, pid, 6, src, dest);
}

static inline uint64_t s3k_supervisor_take_cap(uint64_t sup_cid, uint64_t pid, uint64_t src, uint64_t dest)
{
        return S3K_SYSCALL5(S3K_SYSNR_INVOKE_CAP, sup_cid, pid, 7, src, dest);
}

static inline int s3k_dump_cap(char* buf, int n, cap_t cap)
{
        switch (cap_get_type(cap)) {
        case CAP_TYPE_MEMORY:
                return snprintf(buf, n, "MEMORY{begin=0x%lx,end=0x%lx,rwx=%ld,free=0x%lx,pmp=%ld}",
                             cap_memory_get_begin(cap), cap_memory_get_free(cap), cap_memory_get_end(cap),
                             cap_memory_get_rwx(cap), cap_memory_get_pmp(cap));
        case CAP_TYPE_PMP:
                return snprintf(buf, n, "PMP{addr=0x%lx,rwx=%ld}", cap_pmp_get_addr(cap), cap_pmp_get_rwx(cap));
        case CAP_TYPE_TIME:
                return snprintf(buf, n, "TIME{hartid=%ld,begin=%ld,end=%ld,free=%ld,depth=%ld}", cap_time_get_hartid(cap),
                             cap_time_get_begin(cap), cap_time_get_end(cap), cap_time_get_free(cap),
                             cap_time_get_depth(cap));
        case CAP_TYPE_CHANNELS:
                return snprintf(buf, n, "CHANNELS{begin=%ld,end=%ld,free=%ld}", cap_channels_get_begin(cap),
                             cap_channels_get_end(cap), cap_channels_get_free(cap));
        case CAP_TYPE_RECEIVER:
                return snprintf(buf, n, "RECEIVER{channel=%ld}", cap_receiver_get_channel(cap));
        case CAP_TYPE_SENDER:
                return snprintf(buf, n, "SENDER{channel=%ld}", cap_sender_get_channel(cap));
        case CAP_TYPE_SUPERVISOR:
                return snprintf(buf, n, "SUPERVISOR{begin=%ld,end=%ld,free=%ld}", cap_supervisor_get_begin(cap),
                             cap_supervisor_get_end(cap), cap_supervisor_get_free(cap));
        default:
                *buf = '\0';
                return 0;
        }
}
