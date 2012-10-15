OUTPUT_FORMAT(elf64-x86-64)
ENTRY(start)

SECTIONS {

	__baseaddr = .;

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

	__limitaddr = .;
}
