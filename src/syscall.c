// See LICENSE file for copyright and license details.

#include <stdint.h>
#include "proc.h"

void ecall_get_pid(uintptr_t a[8]) {
        current->args[0] = current->pid;
}
