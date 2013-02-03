#include <pinion.h>

extern void outb(uint16_t port, uint8_t val);
extern uint8_t inb(uint16_t port);

void putchar(char c) {
	
	while ((inb(0x3FD) & 0x20) == 0);
	if (c == '\n') outb(0x3F8, '\r');
	outb(0x3F8, c);
}

void puts(const char *s) {
	
	while (*s) {
		putchar(*s++);
	}
}

void putx(uint64_t x) {
	const char *digit = "0123456789ABCDEF";

	for (int i = 15; i >= 0; i--) {
		putchar(digit[(x >> (4 * i)) & 0xF]);
	}
}

void main() {

	while (1) {
		putchar('a');
	}
}
