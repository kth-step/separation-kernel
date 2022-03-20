// See LICENSE file for copyright and license details.

#include "types.h"
#include "proc.h"

register Process *current __asm__("tp");
void ecall_get_pid(uintptr_t a[8]) {
        a[0] = current->pid;
}
