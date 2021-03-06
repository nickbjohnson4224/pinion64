#include <pinion.h>

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);

void putchar(char c) {
	
	while ((inb(0x3FD) & 0x20) == 0);
	
	if (c == '\n') {
		outb(0x3F8, '\r');
	}

	outb(0x3F8, c);
}

void puts(const char *s) {

	for (size_t i = 0; s[i]; i++) {
		putchar(s[i]);
	}
}

void putx(uint64_t x) {
	const char *digit = "0123456789ABCDEF";
	
	for (int i = 7; i >= 0; i--) {
		putchar(digit[(x >> (i * 8 + 4)) & 0xF]);
		putchar(digit[(x >> (i * 8 + 0)) & 0xF]);
	}
}

void main() {

	for (__PINION_memory_map_index i = 0;; i++) {
		struct __PINION_memory_map_region region =
			__PINION_memory_map_read(i);

		putx(region.base);
		puts(" ");
		putx(region.limit);
		puts("\n");

		if (region.base == 0 && region.limit == 0) {
			break;
		}
	}

	for(;;);
}
