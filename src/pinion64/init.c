#include "internal.h"

void debug_ptr(uint64_t ptr) {
	log(DEBUG, "asm says: %p", ptr);
}

void fault(struct tcb *tcb, struct ccb *ccb) {
	
	if (tcb->ivect == 32) {
		static uint64_t tick = 0;
		if (tick % 1000 == 0) log(DEBUG, "tick %d", tick);
		tick++;
		ccb->lapic->eoi = 0;
		return;
	}

	log(ERROR, "fault caught: %p %d %p", tcb, tcb->ivect, tcb->rip);

	if (tcb->ivect == 14) {
		extern uint64_t read_cr2(void);
		log(ERROR, "page fault at addr %p", read_cr2());
		log(ERROR, "translation: %p %p", pcx_get_trans(NULL, read_cr2()), pcx_get_flags(NULL, read_cr2()));
	}

	if (tcb->ivect == 8) {
		log(ERROR, "unhandled interrupt");
		for(;;);
	}

	if (tcb->ivect < 32) {
		log(ERROR, "unhandled fault");
		for(;;);
	}
}

void init(uint64_t loader, struct unfold64_objl *object_list, struct unfold64_mmap *memory_map) {

	// initialize the physical memory manager
	pmm_init(memory_map);

	// initialize paging
	pcx_init();

	// initialize interrupt handling
	idt_init();

	// allocate the CCB for processor 0
	ccb_new();

	// initialize LAPIC timer
	struct ccb *ccb = ccb_get_self();

	ccb->lapic->destination_format  = 0xFFFFFFFF;
	ccb->lapic->logical_destination = (ccb->lapic->logical_destination & 0xFFFFFF) | 1;
	ccb->lapic->lvt_timer           = 0x10000;
	ccb->lapic->lvt_performance_monitoring_counters = 0x400;
	ccb->lapic->lvt_lint0           = 0x10000;
	ccb->lapic->lvt_lint1           = 0x10000;
	ccb->lapic->task_priority       = 0;

	ccb->lapic->spurious_interrupt_vector  = 33 | 0x100;
	ccb->lapic->timer_initial_count        = 100000; // roughly 1 KHz
	ccb->lapic->lvt_timer                  = 32 | 0x20000;
	ccb->lapic->timer_divide_configuration = 3; // 16

	// allocate initial thread TCB and load
	ccb_load_tcb(tcb_new());

	// load kernel image
	load_kernel(object_list);
}
