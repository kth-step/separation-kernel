/* See LICENSE file for copyright and license details. */
OUTPUT_ARCH("riscv")

ENTRY(_start)

SECTIONS {
    .text : {
        KEEP(*(.init))
        *(.text)
        *(.text.*)
    }

    .data : ALIGN(4) {
        __global_pointer$ = . + 0x7bc;
        *(.sdata)
        *(.sdata.*)
        *(.srodata)
        *(.srodata.*)
    }

    .bss : ALIGN(4) {
        PROVIDE(_bss = .);
        *(.sbss)
        *(.sbss.*)
        *(.bss)
        *(.bss.*)
        *(COMMON)
        PROVIDE(_ebss = ALIGN(4));
    }

    .data : ALIGN(4) {
        *(.data)
        *(.data.*)
        *(.rdata)
        *(.rdata.*)
        *(.rodata)
        *(.rodata.*)
    }
    
    PROVIDE(init_proc_start = ALIGN(4096));
}
