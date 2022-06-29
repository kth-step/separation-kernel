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
	
        .data : ALIGN(8) {
		__global_pointer$ = . + 0x7bc;
		*(.sdata) 
		*(.sdata.*) 
		*(.srodata) 
		*(.srodata.*) 
        } >RAM

	.bss : ALIGN(8) {
                PROVIDE(_sbss = .);
		*(.sbss) 
		*(.sbss.*) 
                . = ALIGN(8);
                PROVIDE(_esbss = .);
	} >RAM

	.data : ALIGN(8) {
		*(.data) 
		*(.data.*) 
		*(.rdata) 
		*(.rdata.*) 
		*(.rodata) 
		*(.rodata.*) 
	} >RAM
	
	.bss : ALIGN(8) {
                PROVIDE(_bss = .);
		*(.bss) 
		*(.bss.*) 
                *(COMMON)
                . = ALIGN(8);
                PROVIDE(_ebss = .);
	} >RAM

        PROVIDE(_heap = ALIGN(4096));
        PROVIDE(_eheap = _heap + 0x8000);
}

