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

int main() {

	puts("hello, world!\n");

	puts(string);

	__PINION_thread_yield();

	__PINION_page_set(0x10000, 0x1000000, 
		__PINION_PAGE_VALID | __PINION_PAGE_READ, 0x2000);
	
	struct __PINION_page_content page = __PINION_page_get(0x11000);

	putx(page.flags);
	puts(" ");
	putx(page.paddr);
	puts("\n");

	puts("that was fun!\n");

	for(;;);

	return 0;
}
