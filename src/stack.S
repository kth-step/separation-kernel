// See LICENSE file for copyright and license details.
#include "platform.h"
.globl stack

.section .bss.stack
.fill (N_CORES * STACK_SIZE)
stack:
