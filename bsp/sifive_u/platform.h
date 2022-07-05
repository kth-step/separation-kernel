#pragma once

/* Number of cores */
#define N_CORES 5
/* Number of PMP registers in hardware */
#define N_PMP 8
/* Ticks per second */
#define TICKS_PER_SECOND 1000000UL
/* Minimum hartid usable by the kernel */
#define MIN_HARTID 1
/* Maximum hartid */
#define MAX_HARTID 4

#ifndef __ASSEMBLER__
#define MTIME ((volatile unsigned long long *)0x200bff8UL)
#define MTIMECMP(x) ((volatile unsigned long long *)(0x2004000UL + ((x)*8)))

static inline unsigned long long read_time(void) {
        return *MTIME;
}

static inline void write_time(unsigned long long time) {
        *MTIME = time;
}

static inline unsigned long long read_timeout(int hartid) {
        return *MTIMECMP(hartid);
}

static inline void write_timeout(int hartid, unsigned long long timeout) {
        *MTIMECMP(hartid) = timeout;
}

static inline int uart_putchar(char c) {
        unsigned int *txdata = (unsigned int *)0x10010000;
        asm volatile(
            "1:amoswap.w t0,%0,(%1)\n"
            "  sext.w t0,t0\n"
            "  bltz t0,1b" ::"r"(c), "r"(txdata)
            : "t0");
        return c;
}
#endif
