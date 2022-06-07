// See LICENSE file for copyright and license details.
#pragma once

#include <stdint.h>

#include "syscall_nr.h"

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
        data[0] = a1;
        data[1] = a2;
        return a0 > 0;
}

uint64_t S3K_MOVE_CAP(uint64_t src, uint64_t dest) {
        return S3K_SYSCALL2(SYSNR_MOVE_CAP, src, dest);
}

uint64_t S3K_DELETE_CAP(uint64_t cid) {
        return S3K_SYSCALL1(SYSNR_DELETE_CAP, cid);
}

uint64_t S3K_REVOKE_CAP(uint64_t cid) {
        return S3K_SYSCALL1(SYSNR_REVOKE_CAP, cid);
}

uint64_t S3K_SLICE_MEMORY(uint64_t src, uint64_t dest, uint64_t begin,
                          uint64_t end, uint64_t rwx) {
        return S3K_SYSCALL5(SYSNR_MS_SLICE, src, dest, begin, end, rwx);
}

uint64_t S3K_SPLIT_MEMORY(uint64_t src, uint64_t dest0, uint64_t dest1,
                          uint64_t mid) {
        return S3K_SYSCALL4(SYSNR_MS_SPLIT, src, dest0, dest1, mid);
}

uint64_t S3K_MAKE_PMP(uint64_t cid_ms, uint64_t cid_pmp, uint64_t addr,
                      uint64_t rwx) {
        return S3K_SYSCALL4(SYSNR_MS_INSTANCIATE, cid_ms, cid_pmp, addr, rwx);
}

uint64_t S3K_LOAD_PMP(uint64_t cid_pmp, uint64_t index) {
        return S3K_SYSCALL2(SYSNR_PE_LOAD, cid_pmp, index);
}

uint64_t S3K_UNLOAD_PMP(uint64_t cid_pmp) {
        return S3K_SYSCALL1(SYSNR_PE_UNLOAD, cid_pmp);
}

uint64_t S3K_SLICE_TIME(uint64_t src, uint64_t dest, uint64_t begin,
                        uint64_t end, uint64_t tsid, uint64_t tsid_end) {
        return S3K_SYSCALL6(SYSNR_TS_SLICE, src, dest, begin, end, tsid,
                            tsid_end);
}

uint64_t S3K_SPLIT_TIME(uint64_t src, uint64_t dest0, uint64_t dest1,
                        uint64_t mid_quantum, uint64_t mid_tsid) {
        return S3K_SYSCALL5(SYSNR_TS_SPLIT, src, dest0, dest1, mid_quantum,
                            mid_tsid);
}

static inline uint64_t S3K_SLICE_CHANNELS(uint64_t src, uint64_t dest,
                                          uint64_t begin, uint64_t end) {
        return S3K_SYSCALL4(SYSNR_CH_SLICE, src, dest, begin, end);
}

static inline uint64_t S3K_SPLIT_CHANNELS(uint64_t src, uint64_t dest0,
                                          uint64_t dest1, uint64_t mid) {
        return S3K_SYSCALL4(SYSNR_CH_SPLIT, src, dest0, dest1, mid);
}

static inline uint64_t S3K_INSTANCIATE_CHANNEL(uint64_t src, uint64_t dest0, uint64_t dest1, uint64_t channel) {
        return -1;
}
static inline uint64_t S3K_MAKE_SENDER(uint64_t cid_send, uint64_t cid_recv) {
        return -1;
}

static inline uint64_t S3K_RECV(uint64_t cid_recv, uint64_t dest_caps,
                                uint64_t msg[4]) {
        return -1;
}
static inline uint64_t S3K_SEND(uint64_t cid_send, uint64_t send_caps,
                                uint64_t msg[4]) {
        return -1;
}

static inline uint64_t S3K_HALT(uint64_t cid_sup) {
        return S3K_SYSCALL1(SYSNR_SP_HALT, cid_sup);
}

static inline uint64_t S3K_RESUME(uint64_t cid_sup) {
        return S3K_SYSCALL1(SYSNR_SP_RESUME, cid_sup);
}

static inline uint64_t S3K_GIVE_CAP(uint64_t cid_sup, uint64_t cid_dest,
                                    uint64_t cid_src) {
        return S3K_SYSCALL3(SYSNR_SP_GIVE, cid_sup, cid_src, cid_dest);
}

static inline uint64_t S3K_TAKE_CAP(uint64_t cid_sup, uint64_t cid_dest,
                                    uint64_t cid_src) {
        return S3K_SYSCALL3(SYSNR_SP_TAKE, cid_sup, cid_src, cid_dest);
}

static inline uint64_t S3K_SUP_READ_CAP(uint64_t cid_sup, uint64_t cid) {
        return S3K_SYSCALL2(SYSNR_SP_TAKE, cid_sup, cid);
}

static inline uint64_t S3K_RESET(uint64_t cid_sup, uint64_t cid_pmp) {
        return S3K_SYSCALL2(SYSNR_SP_RESET, cid_sup, cid_pmp);
}
