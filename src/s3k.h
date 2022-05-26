#pragma once

#include <stdint.h>

static inline uint64_t S3K_GET_PID(void);
static inline uint64_t S3K_READ_CAP(uint64_t cid, uint64_t data[2]);
static inline uint64_t S3K_MOVE_CAP(uint64_t dest, uint64_t src);
static inline uint64_t S3K_DELETE_CAP(uint64_t cid);
static inline uint64_t S3K_REVOKE_CAP(uint64_t cid);
static inline uint64_t S3K_SLICE_MEMORY(uint64_t dest, uint64_t src,
                                        uint64_t begin, uint64_t end,
                                        uint64_t rwx);
static inline uint64_t S3K_SPLIT_MEMORY(uint64_t dest0, uint64_t dest1,
                                        uint64_t src, uint64_t mid);
static inline uint64_t S3K_MAKE_PMP(uint64_t cid_pmp, uint64_t cid_ms,
                                    uint64_t addr, uint64_t rwx);
static inline uint64_t S3K_LOAD_PMP(uint64_t cid_pmp, uint64_t index);
static inline uint64_t S3K_UNLOAD_PMP(uint64_t cid_pmp);
static inline uint64_t S3K_SLICE_TIME(uint64_t dest, uint64_t src,
                                      uint64_t begin, uint64_t end,
                                      uint64_t tsid, uint64_t fuel);
static inline uint64_t S3K_SPLIT_TIME(uint64_t dest0, uint64_t dest1,
                                      uint64_t src, uint64_t mid_quantum,
                                      uint64_t mid_tsid);

static inline uint64_t S3K_SLICE_CHANNELS(uint64_t cid_dest, uint64_t cid_src,
                                          uint64_t begin, uint64_t end);
static inline uint64_t S3K_SPLIT_CHANNELS(uint64_t cid_dest0,
                                          uint64_t cid_dest1, uint64_t cid_src,
                                          uint64_t mid);

static inline uint64_t S3K_MAKE_RECEIVER(uint64_t cid_recv, uint64_t cid_ch,
                                         uint64_t chid);
static inline uint64_t S3K_MAKE_SENDER(uint64_t cid_send, uint64_t cid_recv);

static inline uint64_t S3K_RECV(uint64_t cid_recv, uint64_t dest_caps,
                                uint64_t msg[4]);
static inline uint64_t S3K_SEND(uint64_t cid_send, uint64_t send_caps,
                                uint64_t msg[4]);

uint64_t S3K_GET_PID(void) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        t0 = 0;
        __asm__ volatile("ecall" : "=r"(a0) : "r"(t0));
        return a0;
}

uint64_t S3K_READ_CAP(uint64_t cid, uint64_t data[2]) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        t0 = 1;
        a0 = cid;
        __asm__ volatile("ecall" : "+r"(a0), "=r"(a1), "=r"(a2) : "r"(t0));
        data[0] = a1;
        data[1] = a2;
        return a0;
}

uint64_t S3K_MOVE_CAP(uint64_t dest, uint64_t src) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        t0 = 2;
        a0 = dest;
        a1 = src;
        __asm__ volatile("ecall" : "+r"(a0), "=r"(a1) : "r"(t0));
        return a0;
}

uint64_t S3K_DELETE_CAP(uint64_t cid) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        t0 = 3;
        a0 = cid;
        __asm__ volatile("ecall" : "+r"(a0) : "r"(t0));
        return a0;
}

uint64_t S3K_REVOKE_CAP(uint64_t cid) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        t0 = 4;
        a0 = cid;
        __asm__ volatile("ecall" : "+r"(a0) : "r"(t0));
        return a0;
}

uint64_t S3K_SLICE_MEMORY(uint64_t dest, uint64_t src, uint64_t begin,
                          uint64_t end, uint64_t rwx) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        register uint64_t a3 __asm__("a3");
        register uint64_t a4 __asm__("a4");
        t0 = 5;
        a0 = dest;
        a1 = src;
        a2 = begin;
        a3 = end;
        a4 = rwx;
        __asm__ volatile("ecall"
                         : "+r"(a0)
                         : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(t0));
        return a0;
}

uint64_t S3K_SPLIT_MEMORY(uint64_t dest0, uint64_t dest1, uint64_t src,
                          uint64_t mid) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        register uint64_t a3 __asm__("a3");
        t0 = 6;
        a0 = dest0;
        a1 = dest1;
        a2 = src;
        a3 = mid;
        __asm__ volatile("ecall"
                         : "+r"(a0)
                         : "r"(a1), "r"(a2), "r"(a3), "r"(t0));
        return a0;
}

uint64_t S3K_MAKE_PMP(uint64_t cid_pmp, uint64_t cid_ms, uint64_t addr,
                      uint64_t rwx) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        register uint64_t a3 __asm__("a3");
        t0 = 7;
        a0 = cid_pmp;
        a1 = cid_ms;
        a2 = addr;
        a3 = rwx;
        __asm__ volatile("ecall"
                         : "+r"(a0)
                         : "r"(a1), "r"(a2), "r"(a3), "r"(t0));
        return a0;
}

uint64_t S3K_LOAD_PMP(uint64_t cid_pmp, uint64_t index) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        t0 = 8;
        a0 = cid_pmp;
        a1 = index;
        __asm__ volatile("ecall" : "+r"(a0) : "r"(a1), "r"(t0));
        return a0;
}

uint64_t S3K_UNLOAD_PMP(uint64_t cid_pmp) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        t0 = 9;
        a0 = cid_pmp;
        __asm__ volatile("ecall" : "+r"(a0) : "r"(t0));
        return a0;
}

uint64_t S3K_SLICE_TIME(uint64_t dest, uint64_t src, uint64_t begin,
                        uint64_t end, uint64_t tsid, uint64_t fuel) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        register uint64_t a3 __asm__("a3");
        register uint64_t a4 __asm__("a4");
        register uint64_t a5 __asm__("a5");
        t0 = 10;
        a0 = dest;
        a1 = src;
        a2 = begin;
        a3 = end;
        a4 = tsid;
        a5 = fuel;
        __asm__ volatile("ecall"
                         : "+r"(a0)
                         : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                           "r"(t0));
        return a0;
}

uint64_t S3K_SPLIT_TIME(uint64_t dest0, uint64_t dest1, uint64_t src,
                        uint64_t mid_quantum, uint64_t mid_tsid) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        register uint64_t a3 __asm__("a3");
        register uint64_t a4 __asm__("a4");
        t0 = 11;
        a0 = dest0;
        a1 = dest1;
        a2 = src;
        a3 = mid_quantum;
        a4 = mid_tsid;
        __asm__ volatile("ecall"
                         : "+r"(a0)
                         : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(t0));
        return a0;
}

static inline uint64_t S3K_SLICE_CHANNELS(uint64_t cid_dest, uint64_t cid_src,
                                          uint64_t begin, uint64_t end) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        register uint64_t a3 __asm__("a3");
        t0 = 12;
        a0 = cid_dest;
        a1 = cid_src;
        a2 = begin;
        a3 = end;
        __asm__ volatile("ecall"
                         : "+r"(a0)
                         : "r"(a1), "r"(a2), "r"(a3), "r"(t0));
        return a0;
}

static inline uint64_t S3K_SPLIT_CHANNELS(uint64_t cid_dest0,
                                          uint64_t cid_dest1, uint64_t cid_src,
                                          uint64_t mid) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        register uint64_t a3 __asm__("a3");
        t0 = 13;
        a0 = cid_dest0;
        a1 = cid_dest1;
        a2 = cid_src;
        a3 = mid;
        __asm__ volatile("ecall"
                         : "+r"(a0)
                         : "r"(a1), "r"(a2), "r"(a3), "r"(t0));
        return a0;
}

static inline uint64_t S3K_MAKE_RECEIVER(uint64_t cid_recv, uint64_t cid_ch,
                                         uint64_t chid) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        t0 = 14;
        a0 = cid_recv;
        a1 = cid_ch;
        a2 = chid;
        __asm__ volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(t0));
        return a0;
}
static inline uint64_t S3K_MAKE_SENDER(uint64_t cid_send, uint64_t cid_recv) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        t0 = 15;
        a0 = cid_send;
        a1 = cid_recv;
        __asm__ volatile("ecall" : "+r"(a0) : "r"(a1), "r"(t0));
        return a0;
}

static inline uint64_t S3K_RECV(uint64_t cid_recv, uint64_t dest_caps,
                                uint64_t msg[4]) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        register uint64_t a3 __asm__("a3");
        register uint64_t a4 __asm__("a4");
        register uint64_t a5 __asm__("a5");
        t0 = 16;
        a0 = cid_recv;
        a1 = dest_caps;
        __asm__ volatile("ecall"
                         : "+r"(a0), "=r"(a2), "=r"(a3), "=r"(a4), "=r"(a5)
                         : "r"(a1), "r"(t0));
        msg[0] = a2;
        msg[1] = a3;
        msg[2] = a4;
        msg[3] = a5;
        return a0;
}
static inline uint64_t S3K_SEND(uint64_t cid_send, uint64_t send_caps,
                                uint64_t msg[4]) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        register uint64_t a3 __asm__("a3");
        register uint64_t a4 __asm__("a4");
        register uint64_t a5 __asm__("a5");
        t0 = 17;
        a0 = cid_send;
        a1 = send_caps;
        a2 = msg[0];
        a3 = msg[1];
        a3 = msg[2];
        a3 = msg[3];
        __asm__ volatile("ecall"
                         : "+r"(a0)
                         : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5),
                           "r"(t0));
        return a0;
}

static inline uint64_t S3K_HALT(uint64_t cid_sup) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        t0 = 18;
        a0 = cid_sup;
        __asm__ volatile("ecall" : "+r"(a0) : "r"(t0));
        return a0;
}

static inline uint64_t S3K_RESUME(uint64_t cid_sup) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        t0 = 19;
        a0 = cid_sup;
        __asm__ volatile("ecall" : "+r"(a0) : "r"(t0));
        return a0;
}
static inline uint64_t S3K_GIVE_CAP(uint64_t cid_sup, uint64_t cid_dest,
                                    uint64_t cid_src) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        t0 = 20;
        a0 = cid_sup;
        a1 = cid_dest;
        a2 = cid_src;
        __asm__ volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(t0));
        return a0;
}

static inline uint64_t S3K_TAKE_CAP(uint64_t cid_sup, uint64_t cid_dest,
                                    uint64_t cid_src) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        t0 = 21;
        a0 = cid_sup;
        a1 = cid_dest;
        a2 = cid_src;
        __asm__ volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(t0));
        return a0;
}

static inline uint64_t S3K_SUP_READ_CAP(uint64_t cid_sup, uint64_t cid) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        t0 = 22;
        a0 = cid_sup;
        a1 = cid;
        __asm__ volatile("ecall" : "+r"(a0) : "r"(a1), "r"(t0));
        return a0;
}

static inline uint64_t S3K_RESET(uint64_t cid_sup, uint64_t cid_pmp) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        t0 = 23;
        a0 = cid_sup;
        a1 = cid_pmp;
        __asm__ volatile("ecall" : "+r"(a0) : "r"(a1), "r"(t0));
        return a0;
}
