/* See LICENSE file for copyright and license details. */
OUTPUT_ARCH("riscv")

ENTRY(_start)

/* hardware registers */
mtime = 0x2000000;
mtimecmp = 0x2004000;

MEMORY { 
	RAM(rwx) : ORIGIN = 0x80000000, LENGTH = 0x8000
}

SECTIONS {
	.text : {
		KEEP(*(.init))
		*(.text) 
		*(.text.*)
	} >RAM
	
	.data : {
		__global_pointer$ = . + 0x800;
                *(trap_table)
		*(.data) 
		*(.data.*) 
		*(.rodata) 
		*(.rodata.*)
	} >RAM
	
	.bss : {
		. = ALIGN(64 / 8);
		*(.bss) 
		*(.bss.*)
	}
	
	PROVIDE(__stack_size = 2048);
	.stack : {
		. = ALIGN(16) + __stack_size;
		_sp = .;
	} >RAM
}

