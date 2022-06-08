#pragma once
#ifndef NDEBUG
#include <stdio.h>

#define ASSERT(val)                                                         \
        ({                                                                  \
                if (!(val)) {                                               \
                        printf("Assert violated in function %s (%s:%d).\n", \
                               __FUNCTION__, __FILE__, __LINE__);           \
                        while (1)                                           \
                                ;                                           \
                }                                                           \
        })
#else
#define ASSERT(val)
#endif
