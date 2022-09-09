/* See LICENSE file for copyright and license details. */
OUTPUT_ARCH("riscv")

ENTRY(_start)

MEMORY {
    RAM (rwx) : ORIGIN = 0x80000000, LENGTH = 0x80000000
}

SECTIONS {
    .text : {
        KEEP(*(.init))
        *(.text)
        *(.text.*)
    } >RAM

    .data : ALIGN(4) {
        __global_pointer$ = . + 0x7bc;
        *(.sdata)
        *(.sdata.*)
        *(.srodata)
        *(.srodata.*)
    } >RAM

    .bss : ALIGN(4) {
        PROVIDE(_bss = .);
        *(.sbss)
        *(.sbss.*)
        *(.bss)
        *(.bss.*)
        *(COMMON)
        PROVIDE(_ebss = ALIGN(4));
    } >RAM

    .data : ALIGN(4) {
        *(.data)
        *(.data.*)
        *(.rdata)
        *(.rdata.*)
        *(.rodata)
        *(.rodata.*)
    } >RAM

    PROVIDE(_heap = ALIGN(4096));
    PROVIDE(_eheap = _heap + 0x8000);
}
