/* See LICENSE file for copyright and license details. */
OUTPUT_ARCH("riscv")

ENTRY(_start)

SECTIONS {
    .text : {
        KEEP(*(.text.init))
        *(.text.*)
    }
}
