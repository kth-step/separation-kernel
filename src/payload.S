// See LICENSE file for copyright and license details.
.globl root_payload

.section .data.payload
root_payload:
.incbin "../build/root/root.bin"

