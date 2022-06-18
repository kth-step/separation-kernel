#pragma once
#ifndef NDEBUG

void _assert_fail(const char *, const char *, unsigned int, const char *)
    __attribute__((noreturn));

#define kassert(val)                                                          \
        ({                                                                    \
                if (!(val))                                                   \
                        _assert_fail(#val, __FILE__, __LINE__, __FUNCTION__); \
        })
#else
#define kassert(val)                             \
        ({                                       \
                if (!(val))                      \
                        __builtin_unreachable(); \
        })
#endif
