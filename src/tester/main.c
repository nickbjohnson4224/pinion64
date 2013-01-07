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

const char *string = "foobar\n";

int main() {

	puts("hello, world!\n");

	puts(string);

	pinion_thread_yield();

	puts("that was fun!\n");

	for(;;);

	return 0;
}
