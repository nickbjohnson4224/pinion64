OUTPUT_FORMAT(elf64-x86-64)
ENTRY(main)

SECTIONS {

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
}
