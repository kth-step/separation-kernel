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

    _etext = .; PROVIDE(etext = .);
    .data : ALIGN(8) {
        __DATA_BEGIN__ = .;
        *(.rdata*)
        *(.rodata*)
        *(.data*)
    } >RAM 
    .sdata : ALIGN(8) {
        __SDATA_BEGIN__ = .;
        *(.srdata*)
        *(.srodata*)
        *(.sdata*)
    } >RAM 
    _edata = .; PROVIDE (edata = .);
    .sbss : ALIGN(8) {
        *(.sbss .sbss.*)
    } >RAM 
    .bss : ALIGN(8) {
        *(.bss .bss.*)
        . = ALIGN(8);
    } >RAM 
    __BSS_END__ = .;
    _end = .; PROVIDE (end = .);

    __global_pointer$ = MIN(__SDATA_BEGIN__ + 0x800,
                            MAX(__DATA_BEGIN__ + 0x800, __BSS_END__ - 0x800));

    .payload : ALIGN(4096) {
        _init_payload = .;
        *(.payload)
    } >RAM
}
