#include "internal.h"

void debug_ptr(uint64_t ptr) {
	log(DEBUG, "asm says: %p", ptr);
}

void fault(struct tcb *thread) {
	
	log(ERROR, "fault caught: %p %d %p %p", thread, thread->ivect, thread->rip);

	if (thread->ivect == 14) {
		extern uint64_t read_cr2(void);
		log(ERROR, "page fault at addr %p", read_cr2());
	}

	if (thread->ivect == 8) {
		log(ERROR, "unhandled interrupt");
		for(;;);
	}

	if (thread->ivect < 32) {
		log(ERROR, "unhandled fault");
		for(;;);
	}
}

void apifunc_thread_yield(void) {
	log(DEBUG, "yield!");
}
