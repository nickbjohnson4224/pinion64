#include "internal.h"

struct tcb *fault(struct tcb *thread) {
	
	log(ERROR, "fault caught: %d", thread->ivect);
	
	if (thread->ivect == 8) {
		log(ERROR, "double fault");
		for(;;);
	}

	return thread;
}

void init(uintptr_t baseaddr) {

	log(INIT, "pinion64 starting...");
	log(INFO, "loaded at base address %p", baseaddr);
}
