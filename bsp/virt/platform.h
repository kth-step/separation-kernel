#pragma once

/* Number of cores */
#define N_CORES 5
/* Number of PMP registers in hardware */
#define N_PMP 8
/* Ticks per second */
#define TICKS_PER_SECOND 10000000UL
/* Minimum hartid usable by the kernel */
#define MIN_HARTID 1
/* Maximum hartid */
#define MAX_HARTID 4

#define N_HARTS (MAX_HARTID - MIN_HARTID + 1)

/* Clint memory location */
#define CLINT 0x2000000ull

/* PMP region of boot (>> 10) */
#define PMP 0x8007f, 0x7
/* Memory slices (>> 12) */
#define MEMORY_SLICES        \
    {                        \
        {0x80000, 0x100000}, \
        {                    \
            0x10000, 0x10001 \
        }                    \
    }

/* Stack size. */
/* log_2 of stack size. */
#define LOG_STACK_SIZE 10
#define STACK_SIZE (1UL << LOG_STACK_SIZE)

#define PLATFORM_NAME "virt"

#ifndef __ASSEMBLER__
#define MTIME ((volatile unsigned long long*)0x200bff8UL)
#define MTIMECMP(x) ((volatile unsigned long long*)(0x2004000UL + ((x)*8)))

static inline unsigned long long read_time(void)
{
    return *MTIME;
}

static inline void write_time(unsigned long long time)
{
    *MTIME = time;
}

static inline unsigned long long read_timeout(int hartid)
{
    return *MTIMECMP(hartid);
}

static inline void write_timeout(int hartid, unsigned long long timeout)
{
    *MTIMECMP(hartid) = timeout;
}

static inline void uart_init(void)
{
    unsigned char* uart = (unsigned char*)0x10000000;
    uart[3] = 3;
    uart[2] = 1;
}

static inline int uart_putchar(char c)
{
    unsigned char* uart = (unsigned char*)0x10000000;
    uart[0] = c;
    return c;
}

static inline int uart_getchar(void)
{
    unsigned char* uart = (unsigned char*)0x10000000;
    if (uart[5] & 1)
        return uart[0];
    return 0;
}

#endif /* __ASSEMBLY__ */
