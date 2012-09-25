OUTPUT_FORMAT(elf64-x86-64)
ENTRY(bootstrap)

SECTIONS {
	. = 0xFFFFFFFFC0000000;

	.mboot : AT(ADDR(.mboot) - 0xFFFFFFFFC0000000) {
		*(.mboot)
	}

	.text : AT(ADDR(.text) - 0xFFFFFFFFC0000000) {
		*(.text)
		*(.rodata*)
		. = ALIGN(4096);
	}

	.data : AT(ADDR(.data) - 0xFFFFFFFFC0000000) {
		*(.data)
		. = ALIGN(4096);
	}

	.bss : AT(ADDR(.bss) - 0xFFFFFFFFC0000000) {
		*(COMMON)
		*(.bss)
	}
}
