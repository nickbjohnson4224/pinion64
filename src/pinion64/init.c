#include "internal.h"

void debug_ptr(uint64_t ptr) {
	log(DEBUG, "asm says: %p", ptr);
}

void fault(struct tcb *thread) {
	
	log(ERROR, "fault caught: %p %d %p", thread, thread->ivect, thread->rip);

	if (thread->ivect == 8) {
		log(ERROR, "unhandled interrupt");
		for(;;);
	}

	if (thread->ivect < 32) {
		log(ERROR, "unhandled fault");
		for(;;);
	}
}

void init(uintptr_t baseaddr) {

	log(INIT, "pinion64 starting...");
	log(INFO, "loaded at base address %p", baseaddr);
}

void test(void) {

	struct tcb *tcb = ccb_get_self()->active_tcb;

	log(DEBUG, "current TCB: %p", tcb);

}
