#include "internal.h"

struct tcb *fault(struct tcb *thread) {
	
	log(ERROR, "fault caught: %d %p", thread->ivect, thread->rip);

	if (thread->ivect == 8) {
		log(ERROR, "unhandled interrupt");
		for(;;);
	}

	return thread;
}

void init(uintptr_t baseaddr, void *mmap) {

	log(INIT, "pinion64 starting...");
	log(INFO, "loaded at base address %p", baseaddr);

	pmm_init(mmap);
	pcx_init();

	log(DEBUG, "%p -> %p", mmap, pcx_get_flags(NULL, (uint64_t) mmap));
}
