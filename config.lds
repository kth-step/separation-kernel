/* See LICENSE file for copyright and license details. */
OUTPUT_ARCH("riscv")

ENTRY(_start)

SECTIONS {
    . = 0x80000000;
    .text : {
        _text = .;
        *(.init)
        *(.text)
        *(.text.*)
        _etext = .;
    }

    .data : ALIGN(4K) {
        _data = .;
        *(.data*)
        *(.rdata .rdata.*)
        *(.rodata .rodata.*)
        *(COMMON)
        _sdata = .;
        *(.sdata .sdata.*)
        *(.srdata .srdata.*)
        *(.srodata .srodata*)
        . = ALIGN(8);
        _edata = .;
    }

    .bss : ALIGN(8) {
        _bss = .;
        _sbss = .;
        *(.sbss)
        *(.sbss.*)
        *(.bss)
        *(.bss.*)
        . = ALIGN(8);
        _ebss = .;
    } 

    __global_pointer$ = MIN(_sbss + 0x800, MAX(_data + 0x800, _ebss - 0x800));

    .payload : ALIGN(16K) {
        _payload = .;
        *(.payload)
    }
}
