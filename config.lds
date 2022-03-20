/* See LICENSE file for copyright and license details. */
OUTPUT_ARCH("riscv")

ENTRY(_start)

mtimecmp = 0x2004000;
mtime = 0x200bff8;

MEMORY { 
	RAM(rwx) : ORIGIN = 0x80000000, LENGTH = 2000M
}

SECTIONS {
	.text : {
		KEEP(*(.init))
		*(.text) 
		*(.text.*) 
	} >RAM
	
	.data : {
		. = ALIGN(4096);
		__global_pointer$ = . + 0x800;
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
	
	.bss : {
		. = ALIGN(4096);
		*(.bss) 
		. = ALIGN(4096);
		*(.bss.user_data) 
		*(.bss.*) 
                *(COMMON)
	} >RAM
        _heap = ALIGN(4096);
        _eheap = ORIGIN(RAM) + LENGTH(RAM);
}

