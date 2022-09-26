// See LICENSE file for copyright and license details.
.globl stack

.section .bss.stack
.fill ((MAX_HARTID - MIN_HARTID + 1) * STACK_SIZE)
stack:
