#include "internal.h"

void debug_ptr(uint64_t ptr) {
	log(DEBUG, "asm says: %p", ptr);
}

struct tcb *fault(struct tcb *thread) {
	
	log(ERROR, "fault caught: %p %d %p", thread, thread->ivect, thread->rip);

	if (thread->ivect == 8) {
		log(ERROR, "unhandled interrupt");

		log(DEBUG, "unloaded TCB %p", ccb_unload_tcb());
		for(;;);
	}

	if (thread->ivect < 32) {
		log(ERROR, "unhandled fault");
		for(;;);
	}

	return thread;
}

void init(uintptr_t baseaddr) {

	log(INIT, "pinion64 starting...");
	log(INFO, "loaded at base address %p", baseaddr);
}
