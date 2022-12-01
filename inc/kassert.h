// See LICENSE file for copyright and license details.
#pragma once

#ifndef NDEBUG
#define _STR(x) #x
#define STR(x) _STR(x)
#include "kprint.h"
extern void hang(void) __attribute__((noreturn));
#define kassert(val)                                                                           \
        do {                                                                                   \
                if (!(val)) {                                                                  \
                        kprintf("Assert '%s' failed at %s:%d.\r\n", #val, __FILE__, __LINE__); \
                        hang();                                                                \
                }                                                                              \
        } while (0)
#else
#define trace()
#define kassert(val)                             \
        ({                                       \
                if (!(val))                      \
                        __builtin_unreachable(); \
        })
#endif
