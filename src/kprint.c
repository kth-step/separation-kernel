#include "kprint.h"

#include <stdarg.h>

#include "platform.h"
#include "kassert.h"

void kprint(const char *s) {
        while (*s != '\0') {
                uart_putchar(*s++);
        }
}

static inline void output64(long long n, int sig, int base) {
        if (n == 0) {
                uart_putchar('0');
                return;
        }
        char buff[64];
        buff[63] = '\0';
        unsigned long long i = n;
        int neg = 0;
        if (n < 0 && sig) {
                neg = 1;
                i = -n;
        }
        char *c = &buff[63];
        while (i != 0) {
                unsigned long long j = i % base;
                if (j >= 10)
                        *--c = 'a' + j - 10;
                else
                        *--c = '0' + j;
                i /= base;
        }
        if (neg) {
                *--c = '-';
        }
        kprint(c);
}

static inline void output32(int n, int sig, int base) {
        if (n == 0) {
                uart_putchar('0');
                return;
        }
        char buff[64];
        buff[63] = '\0';
        unsigned int i = n;
        int neg = 0;
        if (n < 0 && sig) {
                neg = 1;
                i = -n;
        }
        char *c = &buff[63];
        while (i != 0) {
                unsigned int j = i % base;
                if (j >= 10)
                        *--c = 'a' + j - 10;
                else
                        *--c = '0' + j;
                i /= base;
        }
        if (neg) {
                *--c = '-';
        }
        kprint(c);
}

void kprintf(const char *format, ...) {
        va_list argp;
        va_start(argp, format);
        for (const char *f = format; *f != '\0'; ++f) {
                if (*f != '%') {
                        uart_putchar(*f);
                        continue;
                }
                switch (*(++f)) {
                        case 'd':
                                output32(va_arg(argp, int), 1, 10);
                                break;
                        case 'u':
                                output32(va_arg(argp, int), 0, 10);
                                break;
                        case 'x':
                                output32(va_arg(argp, int), 0, 16);
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
                                                    va_arg(argp, long long), 1,
                                                    10);
                                                break;
                                        case 'u':
                                                output64(
                                                    va_arg(argp, long long), 0,
                                                    10);
                                                break;
                                        case 'x':
                                                output64(
                                                    va_arg(argp, long long), 0,
                                                    16);
                                                break;
                                }
                                break;
                }
        }
}
