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

	for (int i = 16; i >= 0; i--) {
		putchar(digit[(x >> (4 * i)) & 0xF]);
	}
}

const char *string = "foobar\n";

void stuff() {

	puts("happy birthday!\n");

	for(;;) {
//		for (volatile int i = 0; i < 1000; i++);
		puts("c");
	}
}

static uint8_t stackspace[1024];

int main() {

	puts("hello, world!\n");

	struct __PINION_thread_state proto = {
		.rip = (uintptr_t) stuff,
		.rsp = (uintptr_t) &stackspace[1008]
	};

	__PINION_thread_create(&proto);

	proto.rsp = (uintptr_t) &stackspace[500];

	__PINION_thread_create(&proto);

	for(;;) {
//		for (volatile int i = 0; i < 1000; i++);
		puts("p");
	}

	return 0;
}
