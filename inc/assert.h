#pragma once
#ifndef NDEBUG

void _assert_fail(const char *, const char *, unsigned int, const char *)
    __attribute__((noreturn));

#define ASSERT(val)                                                           \
        ({                                                                    \
                if (!(val))                                                   \
                        _assert_fail(#val, __FILE__, __LINE__, __FUNCTION__); \
        })
#else
#define ASSERT(val)
#endif
