OUTPUT_FORMAT(elf64-x86-64)
ENTRY(start)

SECTIONS {

	PROVIDE_HIDDEN(__baseaddr = .);

	.text : {
		*(.text)
		*(.rodata*)
	}

	.data : {
		*(.data)
	}

	.bss : {
		*(COMMON)
		*(.bss)
	}

	PROVIDE_HIDDEN(__limitaddr = .);
}
