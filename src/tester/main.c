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

#define THREADS 10

static uint8_t stackspace[THREADS][512];
static volatile __PINION_thread_id threadalloc[THREADS];

void thread() {

	puts("I'm a thread!\n");

	puts("and now I'm done!\n");

	__PINION_thread_exit();
}

void reaper() {

	puts("reaper starting...\n");

	__PINION_interrupt_set_tap(0, __PINION_INTERRUPT_VECTOR_ZOMBIE);

	while (1) {

		__PINION_interrupt_wait();

		__PINION_thread_id zombie = __PINION_thread_get_zombie();

		puts("zombie: ");
		putx(zombie);
		puts("\n");

//		for (size_t i = 0; i < THREADS; i++) {
//			if (threadalloc[i] == zombie) {
//				threadalloc[i] = 0;
//				break;
//			}
//		}

		__PINION_thread_reap(zombie);

		__PINION_interrupt_reset(__PINION_INTERRUPT_VECTOR_ZOMBIE);
	}

	__PINION_thread_exit();
}

void main() {

	struct __PINION_thread_state proto = {
		.rip = (uintptr_t) reaper,
		.rsp = (uintptr_t) &stackspace[0][512]
	};

	threadalloc[0] = __PINION_thread_create(&proto);

	proto.rip = (uintptr_t) &thread;

	while (1) {

		for (size_t i = 0; i < THREADS; i++) {
			if (threadalloc[i] == 0) {
				proto.rsp = (uintptr_t) &stackspace[i][512];
				threadalloc[i] = __PINION_thread_create(&proto);
				break;
			}
		}
	}

	__PINION_thread_exit();
}
