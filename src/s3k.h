#pragma once

#include <stdint.h>

static inline uint64_t S3K_GET_PID(void);
static inline uint64_t S3K_CAP_READ(uint64_t cid, uint64_t data[2]);
static inline uint64_t S3K_CAP_MOVE(uint64_t dest, uint64_t src);
static inline uint64_t S3K_CAP_DELETE(uint64_t cid);
static inline uint64_t S3K_CAP_REVOKE(uint64_t cid);
static inline uint64_t S3K_MEMORY_SLICE(uint64_t dest, uint64_t src,
                                        uint64_t begin, uint64_t end,
                                        uint64_t rwx);
static inline uint64_t S3K_MEMORY_SPLIT(uint64_t dest0, uint64_t dest1,
                                        uint64_t src, uint64_t mid);
static inline uint64_t S3K_TIME_SLICE(uint64_t dest, uint64_t src,
                                      uint64_t begin, uint64_t end,
                                      uint64_t tsid, uint64_t fuel);
static inline uint64_t S3K_TIME_SPLIT(uint64_t dest0, uint64_t dest1,
                                      uint64_t src, uint64_t mid_quantum,
                                      uint64_t mid_tsid);

uint64_t S3K_GET_PID(void) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        t0 = 0;
        __asm__ volatile("ecall" : "=r"(a0) : "r"(t0));
        return a0;
}

uint64_t S3K_CAP_READ(uint64_t cid, uint64_t data[2]) {
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

uint64_t S3K_CAP_MOVE(uint64_t dest, uint64_t src) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        t0 = 2;
        a0 = dest;
        a1 = src;
        __asm__ volatile("ecall" : "+r"(a0), "=r"(a1) : "r"(t0));
        return a0;
}

uint64_t S3K_CAP_DELETE(uint64_t cid) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        t0 = 3;
        a0 = cid;
        __asm__ volatile("ecall" : "+r"(a0) : "r"(t0));
        return a0;
}

uint64_t S3K_CAP_REVOKE(uint64_t cid) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        t0 = 4;
        a0 = cid;
        __asm__ volatile("ecall" : "+r"(a0) : "r"(t0));
        return a0;
}

uint64_t S3K_MEMORY_SLICE(uint64_t dest, uint64_t src, uint64_t begin,
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

uint64_t S3K_MEMORY_SPLIT(uint64_t dest0, uint64_t dest1, uint64_t src,
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

uint64_t S3K_TIME_SLICE(uint64_t dest, uint64_t src, uint64_t begin,
                        uint64_t end, uint64_t tsid, uint64_t fuel) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        register uint64_t a3 __asm__("a3");
        register uint64_t a4 __asm__("a4");
        register uint64_t a5 __asm__("a5");
        t0 = 7;
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

uint64_t S3K_TIME_SPLIT(uint64_t dest0, uint64_t dest1, uint64_t src,
                        uint64_t mid_quantum, uint64_t mid_tsid) {
        register uint64_t t0 __asm__("t0");
        register uint64_t a0 __asm__("a0");
        register uint64_t a1 __asm__("a1");
        register uint64_t a2 __asm__("a2");
        register uint64_t a3 __asm__("a3");
        register uint64_t a4 __asm__("a4");
        t0 = 8;
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
