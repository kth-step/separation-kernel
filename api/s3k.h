// See LICENSE file for copyright and license details.
#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "s3k_cap.h"
#include "s3k_consts.h"

#define _S3K_SYSCALL8(a0, a1, a2, a3, a4, a5, a6, a7) S3K_SYSCALL(8, a0, a1, a2, a3, a4, a5, a6, a7)
#define _S3K_SYSCALL7(a0, a1, a2, a3, a4, a5, a6) S3K_SYSCALL(7, a0, a1, a2, a3, a4, a5, a6, 0)
#define _S3K_SYSCALL6(a0, a1, a2, a3, a4, a5) S3K_SYSCALL(6, a0, a1, a2, a3, a4, a5, 0, 0)
#define _S3K_SYSCALL5(a0, a1, a2, a3, a4) S3K_SYSCALL(5, a0, a1, a2, a3, a4, 0, 0, 0)
#define _S3K_SYSCALL4(a0, a1, a2, a3) S3K_SYSCALL(4, a0, a1, a2, a3, 0, 0, 0, 0)
#define _S3K_SYSCALL3(a0, a1, a2) S3K_SYSCALL(3, a0, a1, a2, 0, 0, 0, 0, 0)
#define _S3K_SYSCALL2(a0, a1) S3K_SYSCALL(2, a0, a1, 0, 0, 0, 0, 0, 0)
#define _S3K_SYSCALL1(a0) S3K_SYSCALL(1, a0, 0, 0, 0, 0, 0, 0, 0)

static inline uint64_t S3K_SYSCALL(uint64_t argc, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3,
                                   uint64_t arg4, uint64_t arg5, uint64_t arg6, uint64_t arg7)
{
        register uint64_t a0 __asm__("a0") = arg0;
        register uint64_t a1 __asm__("a1") = arg1;
        register uint64_t a2 __asm__("a2") = arg2;
        register uint64_t a3 __asm__("a3") = arg3;
        register uint64_t a4 __asm__("a4") = arg4;
        register uint64_t a5 __asm__("a5") = arg5;
        register uint64_t a6 __asm__("a6") = arg6;
        register uint64_t a7 __asm__("a7") = arg7;
        switch (argc) {
        case 1:
                asm volatile("ecall" : "+r"(a0));
                break;
        case 2:
                asm volatile("ecall" : "+r"(a0) : "r"(a1));
                break;
        case 3:
                asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2));
                break;
        case 4:
                asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3));
                break;
        case 5:
                asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4));
                break;
        case 6:
                asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5));
                break;
        case 7:
                asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6));
                break;
        case 0:
                asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a6), "r"(a7));
                break;
        default:
                while (1)
                        ;
        }
        return a0;
}

static inline uint64_t s3k_get_pid(void)
{
        return _S3K_SYSCALL1(S3K_SYSCALL_GET_PID);
}

static inline uint64_t s3k_read_reg(uint64_t reg)
{
        return _S3K_SYSCALL2(S3K_SYSCALL_READ_REG, reg);
}

static inline uint64_t s3k_write_reg(uint64_t regnr, uint64_t val)
{
        return _S3K_SYSCALL3(S3K_SYSCALL_WRITE_REG, regnr, val);
}

static inline uint64_t s3k_yield()
{
        return _S3K_SYSCALL1(S3K_SYSCALL_YIELD);
}

static inline uint64_t s3k_read_cap(uint64_t cidx, cap_t* cap)
{
        register int64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        a0 = S3K_SYSCALL_READ_CAP;
        a1 = cidx;
        __asm__ volatile("ecall" : "+r"(a0), "+r"(a1), "=r"(a2));
        *cap = (cap_t){a1, a2};
        return a0;
}

static inline uint64_t s3k_move_cap(uint64_t cidx_src, uint64_t cidx_dest)
{
        return _S3K_SYSCALL3(S3K_SYSCALL_MOVE_CAP, cidx_src, cidx_dest);
}

static inline uint64_t s3k_delete_cap(uint64_t cidx)
{
        return _S3K_SYSCALL2(S3K_SYSCALL_DELETE_CAP, cidx);
}

static inline uint64_t s3k_revoke_cap(uint64_t cidx)
{
        return _S3K_SYSCALL2(S3K_SYSCALL_REVOKE_CAP, cidx);
}

static inline uint64_t s3k_derive_cap(uint64_t src_cidx, uint64_t dest_cidx, cap_t cap)
{
        return _S3K_SYSCALL5(S3K_SYSCALL_DERIVE_CAP, src_cidx, dest_cidx, cap.word0, cap.word1);
}

static inline uint64_t s3k_supervisor_suspend(uint64_t sup_cid, uint64_t pid)
{
        return _S3K_SYSCALL3(S3K_SYSCALL_SUP_SUSPEND, sup_cid, pid);
}

static inline uint64_t s3k_supervisor_resume(uint64_t sup_cid, uint64_t pid)
{
        return _S3K_SYSCALL3(S3K_SYSCALL_SUP_RESUME, sup_cid, pid);
}

static inline uint64_t s3k_supervisor_get_state(uint64_t sup_cid, uint64_t pid)
{
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        a0 = S3K_SYSCALL_SUP_GET_STATE;
        a1 = sup_cid;
        a2 = pid;
        __asm__ volatile("ecall" : "+r"(a0), "+r"(a1) : "r"(a2));
        if (a0 != S3K_OK)
                return -1;
        return a1;
}

static inline uint64_t s3k_supervisor_get_reg(uint64_t sup_cid, uint64_t pid, uint64_t reg)
{
        return _S3K_SYSCALL4(S3K_SYSCALL_SUP_GET_REG, sup_cid, pid, reg);
}

static inline uint64_t s3k_supervisor_set_reg(uint64_t sup_cid, uint64_t pid, uint64_t reg, uint64_t val)
{
        return _S3K_SYSCALL5(S3K_SYSCALL_SUP_SET_REG, sup_cid, pid, reg, val);
}

static inline cap_t s3k_supervisor_read_cap(uint64_t sup_cid, uint64_t pid, uint64_t cid)
{
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        register uint64_t a3 __asm__("a3");
        a0 = S3K_SYSCALL_SUP_READ_CAP;
        a1 = sup_cid;
        a2 = pid;
        a3 = cid;
        __asm__ volatile("ecall" : "+r"(a0), "+r"(a1), "+r"(a2) : "r"(a3));
        if (a0 == S3K_OK)
                return (cap_t){a1, a2};
        return NULL_CAP;
}

static inline uint64_t s3k_supervisor_move_cap(uint64_t sup_cid, uint64_t pid, uint64_t take, uint64_t src,
                                               uint64_t dest)
{
        return _S3K_SYSCALL6(S3K_SYSCALL_SUP_MOVE_CAP, sup_cid, pid, take, src, dest);
}

// static inline uint64_t s3k_receive(uint64_t cid, uint64_t msg[4], uint64_t dest)
// {
//         register uint64_t a0 __asm__("a0");
//         register uint64_t a1 __asm__("a1");
//         register uint64_t a2 __asm__("a2");
//         register uint64_t a3 __asm__("a3");
//         register uint64_t a4 __asm__("a4");
//         register uint64_t a5 __asm__("a5");
//         register uint64_t t0 __asm__("t0");
//         a0 = cid;
//         a5 = dest;
//         t0 = S3K_SYSCALL_RECEIVE;
//         __asm__("ecall" : "+r"(a0), "=r"(a1), "=r"(a2), "=r"(a3), "=r"(a4) : "r"(a5), "r"(t0));
//         msg[0] = (a0 == S3K_OK) ? a1 : 0;
//         msg[1] = (a0 == S3K_OK) ? a2 : 0;
//         msg[2] = (a0 == S3K_OK) ? a3 : 0;
//         msg[3] = (a0 == S3K_OK) ? a4 : 0;
//         return a0;
// }
//
// static inline uint64_t s3k_send(uint64_t cid, uint64_t msg[4], uint64_t src)
// {
//         return S3K_SYSCALL6(S3K_SYSCALL_SEND, cid, msg[0], msg[1], msg[2], msg[3], src);
// }
