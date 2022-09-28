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
        *(.data)
        *(.data.*)
        *(.rdata)
        *(.rdata.*)
        *(.rodata)
        *(.rodata.*)
    } >RAM

    .bss : ALIGN(4) {
        PROVIDE(_bss = .);
        *(.sbss)
        *(.sbss.*)
        *(.bss)
        *(.bss.*)
        PROVIDE(_ebss = ALIGN(4));
    } >RAM

    
    .payload : ALIGN(4096) {
        *(.payload)
    } >RAM AT>RAM
}
