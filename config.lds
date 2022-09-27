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

    .data : {
        __global_pointer$ = . + 0x7bc;
        *(.sdata)
        *(.sdata.*)
        *(.srodata)
        *(.srodata.*)
    } >RAM

    .bss : {
        PROVIDE(_bss = .);
        *(.sbss)
        *(.sbss.*)
        *(.bss)
        *(.bss.*)
        *(COMMON)
        PROVIDE(_ebss = ALIGN(4));
    } >RAM

    .data :  {
        *(.data)
        *(.data.*)
        *(.rdata)
        *(.rdata.*)
        *(.rodata)
        *(.rodata.*)
    } >RAM
    
    .data : ALIGN(4096) {
        KEEP(*(.data.payload))
    } >RAM
}
