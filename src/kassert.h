// See LICENSE file for copyright and license details.
#pragma once

#ifndef NDEBUG
#include "kprint.h"
#include "platform.h"

#define kassert(val)                                                                              \
        ({                                                                                        \
                while (!(val)) {                                                                  \
                        kprintf("Assert '%s' failed at %s:%d in function %s\r\n", #val, __FILE__, \
                                __LINE__, __FUNCTION__);                                          \
                }                                                                                 \
        })
#else
#define kassert(val)                             \
        ({                                       \
                if (!(val))                      \
                        __builtin_unreachable(); \
        })
#endif
