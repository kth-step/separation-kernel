// See LICENSE file for copyright and license details.
#include <stdio.h>
#include "inc/lock.h"

void main(uintptr_t pid, uint64_t begin, uint64_t end) {
        static Lock l;
        while (1) {
        acquire_lock(&l);
        printf("%llu\t%llx\t%llx\r\n", pid, begin, end);
        release_lock(&l);
        }
}
