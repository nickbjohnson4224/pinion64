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
	
	__PINION_interrupt_set_tap(0, 0x0101);
	
	while (1) {
		__PINION_interrupt_wait();

		puts("got interrupt\n");

		__PINION_interrupt_reset(0x0101);
	}
}
