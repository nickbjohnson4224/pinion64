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

const char *string = "foobar\n";

void stuff() {

	puts("happy birthday!\n");

	for (volatile int i = 0; i < 50000000; i++);

	puts("goodnight!\n");

	__PINION_thread_exit();

	for (;;);
}

void irqlistener() {
	
	__PINION_thread_set_tap(0, 0x100);
	__PINION_thread_set_tap(1, 0x101);
	__PINION_thread_set_tap(2, 0x102);
	__PINION_thread_set_tap(3, 0x103);

	while (1) {
		__PINION_interrupt_vector v = __PINION_thread_wait();

		puts("got IRQ: ");
		putx(v);
		puts("\n");

		__PINION_thread_reset(v);
	}

	__PINION_thread_exit();
}

static uint8_t stackspace[1024];

void main() {

	puts("hello, world!\n");

	struct __PINION_thread_state proto = {
		.rip = (uintptr_t) irqlistener,
		.rsp = (uintptr_t) &stackspace[1008]
	};

	__PINION_thread_create(&proto);

	__PINION_thread_exit();
}
