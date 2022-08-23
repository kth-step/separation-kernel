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

/* Clint memory location */
#define CLINT 0x2000000ull

/* Stack size. */
/* log_2 of stack size. */
#define LOG_STACK_SIZE 12
#define STACK_SIZE (1UL << LOG_STACK_SIZE)

#define PLATFORM_NAME "sifive_u"

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
        volatile unsigned int *txctrl = (volatile unsigned int *)0x10010008;
        *txctrl = 0x1;
        volatile  int *txdata = (volatile int *)0x10010000;
        while (*txdata < 0);
        *txdata = c;
        /*
        asm volatile(
            "1:amoswap.w t0,%0,(%1)\n"
            "  sext.w t0,t0\n"
            "  bltz t0,1b" ::"r"(c), "r"(txdata)
            : "t0");
            */
        return c;
}
#endif
