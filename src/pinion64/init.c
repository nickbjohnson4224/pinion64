// Copyright (C) 2012-2013 Nick Johnson <nickbjohnson4224 at gmail.com>
//
// Permission to use, copy, modify, and distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
// 
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
// ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
// ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
// OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#include "internal.h"

void debug_ptr(uint64_t ptr) {
	log(DEBUG, "asm says: %p", ptr);
}

void fault(struct tcb *tcb, struct ccb *ccb) {
	
	if (tcb->ivect == 32) {
		ccb->lapic->eoi = 0;

		static int i = 0;
		i++;
		if ((i & 0x3FF) == 0) {
			interrupt_vector_fire(0x101);
		}

		scheduler_schedule();

		return;
	}

	if (tcb->ivect == 0x100) {
	
		if (tcb->rdi == 0 && tcb->state != TCB_STATE_RUNNING) {
			ccb_unload_tcb();
		}

		scheduler_schedule();

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

static struct interrupt_vector_page_vtable irq_vector_page_vtable;

static void irq_on_fire(uint16_t vec) {
	log(DEBUG, "irq %d fired", vec & 0x7F);
}

static void irq_on_reset(uint16_t vec) {
	log(DEBUG, "irq %d reset", vec & 0x7F);
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

	// initialize interrupt routes
	irq_vector_page_vtable.on_fire = irq_on_fire;
	irq_vector_page_vtable.on_reset = irq_on_reset;
	interrupt_add_vector_page(0x0100, &irq_vector_page_vtable);

	// allocate initial thread TCB and add to scheduler
	scheduler_add_tcb(tcb_new());

	// schedule first thread
	scheduler_schedule();

	// load kernel image
	load_kernel(object_list);
}
