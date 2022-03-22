// See LICENSE file for copyright and license details.
#include <stdio.h>

#include "lock.h"

void main(uintptr_t pid, uint64_t begin, uint64_t end) {
        while (1) {
                printf("%llu\t%llx\t%llx\r\n", pid, begin, end);
        }
}
