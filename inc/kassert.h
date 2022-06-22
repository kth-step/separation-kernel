#pragma once
#ifndef NDEBUG
#include <uart.h>

#define TOSTR(c) #c
#define ASSERT_FAIL(a, b, c, d)                                          \
        uart_puts("ASSERT '" #a "' FAILED AT " b ":" TOSTR(c)"\r\n")
#define kassert(val)                                                        \
        ({                                                                  \
                if (!(val)) {                                               \
                        ASSERT_FAIL(val, __FILE__, __LINE__, __FUNCTION__); \
                        while (1)                                           \
                                asm volatile("ebreak");                     \
                }                                                           \
        })
#else
#define kassert(val)                             \
        ({                                       \
                if (!(val))                      \
                        __builtin_unreachable(); \
        })
#endif
