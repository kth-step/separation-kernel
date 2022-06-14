#include "assert.h"

#ifndef NDEBUG
#include <stdio.h>

void _assert_fail(const char *assert, const char *file, unsigned int line,
                  const char *function) {
        printf("Assertion '%s' failed in file %s:%d in function %s\n",
               assert, file, line, function);
        while(1);
}
#endif
