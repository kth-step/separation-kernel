#include "kprint.h"

#include <stdarg.h>

#include "kassert.h"
#include "platform.h"
#include "types.h"

void kprint(const char *s) {
        while (*s != '\0') {
                uart_putchar(*s++);
        }
}

static inline void output64(long long n, int sig, int base, char padding,
                            int padd_width) {
        char buff[64];
        buff[63] = '\0';
        unsigned long long i = n;
        int neg = 0;
        if (n < 0 && sig) {
                padd_width--;
                neg = 1;
                i = -n;
        }
        char *c = &buff[63];
        do {
                unsigned long long j = i % base;
                if (j >= 10)
                        *--c = 'a' + j - 10;
                else
                        *--c = '0' + j;
                i /= base;
                padd_width--;
        } while (i != 0);
        while (padd_width > 0) {
                *--c = padding;
                padd_width--;
        }
        if (neg) {
                *--c = '-';
        }
        kprint(c);
}

static inline void output32(int n, bool sig, int base, char padding,
                            int padd_width) {
        char buff[64];
        buff[63] = '\0';
        unsigned int i = n;
        bool neg = 0;
        if (n < 0 && sig) {
                padd_width--;
                neg = 1;
                i = -n;
        }
        char *c = &buff[63];
        do {
                unsigned int j = i % base;
                if (j >= 10)
                        *--c = 'a' + j - 10;
                else
                        *--c = '0' + j;
                i /= base;
                padd_width--;
        } while (i != 0);
        while (padd_width > 0) {
                *--c = padding;
                padd_width--;
        }
        if (neg) {
                *--c = '-';
        }
        kprint(c);
}

void kprintf(const char *format, ...) {
        static volatile int lock = 0;
        while (!__sync_bool_compare_and_swap(&lock, 0, 1))
                ;
        va_list argp;
        va_start(argp, format);
        for (const char *f = format; *f != '\0'; ++f) {
                if (*f != '%') {
                        uart_putchar(*f);
                        continue;
                }
                f++;
                char padding = ' ';
                int padd_width = 0;
                if (*f == '0') {
                        padding = '0';
                        f++;
                }
                while (*f >= '0' && *f <= '9') {
                        padd_width *= 10;
                        padd_width += *f - '0';
                        f++;
                }
                switch (*f) {
                        case 'd':
                                output32(va_arg(argp, int), true, 10, padding,
                                         padd_width);
                                break;
                        case 'u':
                                output32(va_arg(argp, int), false, 10, padding,
                                         padd_width);
                                break;
                        case 'x':
                                output32(va_arg(argp, int), false, 1, padding,
                                         padd_width);
                                break;
                        case 's':
                                kprint(va_arg(argp, char *));
                                break;
                        case 'c':
                                uart_putchar(va_arg(argp, int));
                                break;
                        case '%':
                                uart_putchar('%');
                                break;
                        case 'l':
                                switch (*(++f)) {
                                        case 'd':
                                                output64(
                                                    va_arg(argp, long long),
                                                    true, 10, padding,
                                                    padd_width);
                                                break;
                                        case 'u':
                                                output64(
                                                    va_arg(argp, long long),
                                                    false, 10, padding,
                                                    padd_width);
                                                break;
                                        case 'x':
                                                output64(
                                                    va_arg(argp, long long),
                                                    false, 16, padding,
                                                    padd_width);
                                                break;
                                }
                                break;
                }
        }
        lock = 0;
}
