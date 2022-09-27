// See LICENSE file for copyright and license details.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "s3k_cap.h"
#include "s3k_consts.h"
#include "snprintf.h"

#define S3K_SYSCALL8(sysnr, a0, a1, a2, a3, a4, a5, a6, a7) S3K_SYSCALL(8, sysnr, a0, a1, a2, a3, a4, a5, a6, a7)
#define S3K_SYSCALL7(sysnr, a0, a1, a2, a3, a4, a5, a6) S3K_SYSCALL(7, sysnr, a0, a1, a2, a3, a4, a5, a6, 0)
#define S3K_SYSCALL6(sysnr, a0, a1, a2, a3, a4, a5) S3K_SYSCALL(6, sysnr, a0, a1, a2, a3, a4, a5, 0, 0)
#define S3K_SYSCALL5(sysnr, a0, a1, a2, a3, a4) S3K_SYSCALL(5, sysnr, a0, a1, a2, a3, a4, 0, 0, 0)
#define S3K_SYSCALL4(sysnr, a0, a1, a2, a3) S3K_SYSCALL(4, sysnr, a0, a1, a2, a3, 0, 0, 0, 0)
#define S3K_SYSCALL3(sysnr, a0, a1, a2) S3K_SYSCALL(3, sysnr, a0, a1, a2, 0, 0, 0, 0, 0)
#define S3K_SYSCALL2(sysnr, a0, a1) S3K_SYSCALL(2, sysnr, a0, a1, 0, 0, 0, 0, 0, 0)
#define S3K_SYSCALL1(sysnr, a0) S3K_SYSCALL(1, sysnr, a0, 0, 0, 0, 0, 0, 0, 0)
#define S3K_SYSCALL0(sysnr) S3K_SYSCALL(0, sysnr, 0, 0, 0, 0, 0, 0, 0, 0)

static inline uint64_t S3K_SYSCALL(uint64_t argc, uint64_t sysnr, uint64_t arg0, uint64_t arg1, uint64_t arg2,
                                   uint64_t arg3, uint64_t arg4, uint64_t arg5, uint64_t arg6, uint64_t arg7)
{
    register uint64_t a0 __asm__("a0") = arg0;
    register uint64_t a1 __asm__("a1") = arg1;
    register uint64_t a2 __asm__("a2") = arg2;
    register uint64_t a3 __asm__("a3") = arg3;
    register uint64_t a4 __asm__("a4") = arg4;
    register uint64_t a5 __asm__("a5") = arg5;
    register uint64_t a6 __asm__("a6") = arg6;
    register uint64_t a7 __asm__("a7") = arg7;
    register uint64_t t0 __asm__("t0") = sysnr;
    switch (argc) {
    case 0:
        asm volatile("ecall" : "=r"(a0) : "r"(t0));
        break;
    case 1:
        asm volatile("ecall" : "+r"(a0) : "r"(t0));
        break;
    case 2:
        asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(t0));
        break;
    case 3:
        asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(t0));
        break;
    case 4:
        asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(t0));
        break;
    case 5:
        asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(t0));
        break;
    case 6:
        asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(t0));
        break;
    case 7:
        asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(t0));
        break;
    case 8:
        asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7), "r"(t0));
        break;
    default:
        while (1)
            ;
    }
    return a0;
}

static inline uint64_t s3k_get_pid(void)
{
    return S3K_SYSCALL0(S3K_SYSNR_GET_PID);
}

static inline uint64_t s3k_read_reg(uint64_t regnr)
{
    return S3K_SYSCALL1(S3K_SYSNR_READ_REG, regnr);
}

static inline uint64_t s3k_write_reg(uint64_t regnr, uint64_t val)
{
    return S3K_SYSCALL2(S3K_SYSNR_WRITE_REG, regnr, val);
}

static inline uint64_t s3k_yield()
{
    return S3K_SYSCALL0(S3K_SYSNR_YIELD);
}

static inline uint64_t s3k_read_cap(uint64_t cidx, cap_t* cap)
{
    register int64_t a0 __asm__("a0");
    register uint64_t a1 __asm__("a1");
    register uint64_t a2 __asm__("a2");
    register uint64_t t0 __asm__("t0");
    a0 = cidx;
    t0 = S3K_SYSNR_READ_CAP;
    __asm__ volatile("ecall" : "+r"(a0), "=r"(a1), "=r"(a2) : "r"(t0));
    *cap = (cap_t){a1, a2};
    return a0;
}

static inline uint64_t s3k_move_cap(uint64_t cidx_src, uint64_t cidx_dest)
{
    return S3K_SYSCALL2(S3K_SYSNR_MOVE_CAP, cidx_src, cidx_dest);
}

static inline uint64_t s3k_delete_cap(uint64_t cidx)
{
    return S3K_SYSCALL1(S3K_SYSNR_DELETE_CAP, cidx);
}

static inline uint64_t s3k_revoke_cap(uint64_t cidx)
{
    return S3K_SYSCALL1(S3K_SYSNR_REVOKE_CAP, cidx);
}

static inline uint64_t s3k_derive_cap(uint64_t src_cidx, uint64_t dest_cidx, cap_t cap)
{
    return S3K_SYSCALL4(S3K_SYSNR_DERIVE_CAP, src_cidx, dest_cidx, cap.word0, cap.word1);
}

static inline uint64_t s3k_supervisor_suspend(uint64_t sup_cid, uint64_t pid)
{
    return S3K_SYSCALL3(S3K_SYSNR_INVOKE_CAP, sup_cid, pid, S3K_SYSNR_INVOKE_SUPERVISOR_SUSPEND);
}

static inline uint64_t s3k_supervisor_resume(uint64_t sup_cid, uint64_t pid)
{
    return S3K_SYSCALL3(S3K_SYSNR_INVOKE_CAP, sup_cid, pid, S3K_SYSNR_INVOKE_SUPERVISOR_RESUME);
}

static inline uint64_t s3k_supervisor_get_state(uint64_t sup_cid, uint64_t pid)
{
    register uint64_t a0 __asm__("a0");
    register uint64_t a1 __asm__("a1");
    register uint64_t a2 __asm__("a2");
    register uint64_t a7 __asm__("a7");
    a0 = sup_cid;
    a1 = pid;
    a2 = 2;
    a7 = S3K_SYSNR_INVOKE_SUPERVISOR_GET_STATE;
    __asm__ volatile("ecall" : "+r"(a0), "+r"(a1) : "r"(a2), "r"(a7));
    if (a0 != S3K_OK)
        return -1;
    return a1;
}

static inline uint64_t s3k_supervisor_read_reg(uint64_t sup_cid, uint64_t pid, uint64_t reg_nr)
{
    return S3K_SYSCALL4(S3K_SYSNR_INVOKE_CAP, sup_cid, pid, S3K_SYSNR_INVOKE_SUPERVISOR_READ_REG, reg_nr);
}

static inline uint64_t s3k_supervisor_write_reg(uint64_t sup_cid, uint64_t pid, uint64_t reg_nr, uint64_t val)
{
    return S3K_SYSCALL5(S3K_SYSNR_INVOKE_CAP, sup_cid, pid, S3K_SYSNR_INVOKE_SUPERVISOR_WRITE_REG, reg_nr, val);
}

static inline cap_t s3k_supervisor_read_cap(uint64_t sup_cid, uint64_t pid, uint64_t cid)
{
    register uint64_t a0 __asm__("a0");
    register uint64_t a1 __asm__("a1");
    register uint64_t a2 __asm__("a2");
    register uint64_t a3 __asm__("a3");
    register uint64_t t0 __asm__("t0");
    a0 = sup_cid;
    a1 = pid;
    a2 = 5;
    a3 = cid;
    t0 = S3K_SYSNR_INVOKE_SUPERVISOR_READ_CAP;
    __asm__ volatile("ecall" : "+r"(a0), "+r"(a1), "+r"(a2) : "r"(a3), "r"(t0));
    if (a0 == S3K_OK)
        return (cap_t){a1, a2};
    return NULL_CAP;
}

static inline uint64_t s3k_supervisor_give_cap(uint64_t sup_cid, uint64_t pid, uint64_t src, uint64_t dest)
{
    return S3K_SYSCALL5(S3K_SYSNR_INVOKE_CAP, sup_cid, pid, S3K_SYSNR_INVOKE_SUPERVISOR_GIVE_CAP, src, dest);
}

static inline uint64_t s3k_supervisor_take_cap(uint64_t sup_cid, uint64_t pid, uint64_t src, uint64_t dest)
{
    return S3K_SYSCALL5(S3K_SYSNR_INVOKE_CAP, sup_cid, pid, S3K_SYSNR_INVOKE_SUPERVISOR_TAKE_CAP, src, dest);
}

static inline uint64_t s3k_receive(uint64_t cid, uint64_t msg[4], uint64_t dest)
{
    register uint64_t a0 __asm__("a0");
    register uint64_t a1 __asm__("a1");
    register uint64_t a2 __asm__("a2");
    register uint64_t a3 __asm__("a3");
    register uint64_t a4 __asm__("a4");
    register uint64_t a5 __asm__("a5");
    register uint64_t t0 __asm__("t0");
    a0 = cid;
    a5 = dest;
    t0 = S3K_SYSNR_INVOKE_CAP;
    __asm__("ecall" : "+r"(a0), "=r"(a1), "=r"(a2), "=r"(a3), "=r"(a4) : "r"(a5), "r"(t0));
    msg[0] = (a0 == S3K_OK) ? a1 : 0;
    msg[1] = (a0 == S3K_OK) ? a2 : 0;
    msg[2] = (a0 == S3K_OK) ? a3 : 0;
    msg[3] = (a0 == S3K_OK) ? a4 : 0;
    return a0;
}

static inline uint64_t s3k_send(uint64_t cid, uint64_t msg[4], uint64_t src)
{
    return S3K_SYSCALL6(S3K_SYSNR_INVOKE_CAP, cid, msg[0], msg[1], msg[2], msg[3], src);
}

static inline int s3k_dump_cap(char* buf, int n, cap_t cap)
{
    switch (cap_get_type(cap)) {
    case CAP_TYPE_EMPTY:
        return snprintf(buf, n, "EMPTY");
    case CAP_TYPE_MEMORY:
        return snprintf(buf, n, "MEMORY{begin=0x%lx,end=0x%lx,rwx=%ld,free=0x%lx,pmp=%ld}", cap_memory_get_begin(cap),
                        cap_memory_get_end(cap), cap_memory_get_rwx(cap), cap_memory_get_free(cap),
                        cap_memory_get_pmp(cap));
    case CAP_TYPE_PMP:
        return snprintf(buf, n, "PMP{addr=0x%lx,rwx=%ld}", cap_pmp_get_addr(cap), cap_pmp_get_rwx(cap));
    case CAP_TYPE_TIME:
        return snprintf(buf, n, "TIME{hartid=%ld,begin=%ld,end=%ld,free=%ld}", cap_time_get_hartid(cap),
                        cap_time_get_begin(cap), cap_time_get_end(cap), cap_time_get_free(cap));
    case CAP_TYPE_CHANNELS:
        return snprintf(buf, n, "CHANNELS{begin=%ld,end=%ld,free=%ld}", cap_channels_get_begin(cap),
                        cap_channels_get_end(cap), cap_channels_get_free(cap));
    case CAP_TYPE_RECEIVER:
        return snprintf(buf, n, "RECEIVER{channel=%ld,mode=%ld}", cap_receiver_get_channel(cap),
                        cap_receiver_get_grant(cap));
    case CAP_TYPE_SENDER:
        return snprintf(buf, n, "SENDER{channel=%ld,mode=%ld}", cap_sender_get_channel(cap), cap_sender_get_grant(cap));
    case CAP_TYPE_SERVER:
        return snprintf(buf, n, "SERVER{channel=%ld,mode=%ld}", cap_server_get_channel(cap), cap_server_get_grant(cap));
    case CAP_TYPE_CLIENT:
        return snprintf(buf, n, "CLIENT{channel=%ld,mode=%ld}", cap_client_get_channel(cap), cap_client_get_grant(cap));
    case CAP_TYPE_SUPERVISOR:
        return snprintf(buf, n, "SUPERVISOR{begin=%ld,end=%ld,free=%ld}", cap_supervisor_get_begin(cap),
                        cap_supervisor_get_end(cap), cap_supervisor_get_free(cap));
    default:
        return snprintf(buf, n, "INVALID");
    }
}
