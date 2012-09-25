OUTPUT_FORMAT(elf32-i386)
ENTRY(start)

SECTIONS {
	. = 0x00100000;

	.mboot : AT(ADDR(.mboot)) {
		*(.mboot)
	}

	.text : AT(ADDR(.text)) {
		*(.text)
		*(.rodata*)
	}

	.data : AT(ADDR(.data)) {
		*(.data)
	}

	.bss : AT(ADDR(.bss)) {
		_sbss = .;
		*(COMMON)
		*(.bss)
		_ebss = .;
	}

	end = .;
}
