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

#include "../include/pinion.h"
#include "internal.h"

void debug_ptr(uint64_t ptr) {
	log(DEBUG, "asm says: %p", ptr);
}

void fault(struct tcb *tcb, struct ccb *ccb) {
	
	if (tcb->ivect == 32) {
		ccb->lapic->eoi = 0;

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

	if (tcb->ivect == 14) {
		extern uint64_t read_cr2(void);
		log(ERROR, "page fault at addr %p (ip %p, sp %p, id %d)", read_cr2(), tcb->rip, tcb->rsp, tcb->id);
		log(ERROR, "translation: %p %p", pcx_get_trans(NULL, read_cr2()), pcx_get_flags(NULL, read_cr2()));

		queue_pagefault(ccb_unload_tcb());
		interrupt_vector_fire(__PINION_INTERRUPT_PAGEFAULT);
		scheduler_schedule();
		return;
	}

	if (tcb->ivect == 8) {
		log(ERROR, "PANIC: double fault");
		for(;;);
	}

	if (tcb->ivect < 32) {
		queue_miscfault(ccb_unload_tcb());
		interrupt_vector_fire(__PINION_INTERRUPT_MISCFAULT);
		scheduler_schedule();
		return;
	}
}

static struct interrupt_vector_page_vtable irq_vector_page_vtable;
static struct interrupt_vector_page_vtable pinion_vector_page_vtable;

static void irq_on_fire(uint16_t vec) {
	log(DEBUG, "irq %d fired", vec & 0x7F);

	// TODO disable IRQ
}

static void irq_on_reset(uint16_t vec) {
	log(DEBUG, "irq %d reset", vec & 0x7F);

	// TODO enable IRQ
}

static void pinion_on_reset(uint16_t vec) {
	
	switch (vec & 0x7F) {
	case 0: // pagefault
		if (has_pagefault()) {
			interrupt_vector_fire(vec);
		}
		break;
	case 1: // miscfault
		if (has_miscfault()) {
			interrupt_vector_fire(vec);
		}
		break;
	case 2: // zombie
		if (has_zombie()) {
			interrupt_vector_fire(vec);
		}
		break;
	}
}

static int strcmp(const char *a, const char *b) {
	
	while (*a && *b) {
		if (*(a++) != *(b++)) return 1;
	}

	return 0;
}

void init(uint64_t loader, struct unfold64_objl *object_list, struct unfold64_mmap *memory_map) {

	// parse configuration
	for (size_t i = 0; i < object_list->count; i++) {
		if (!strcmp(object_list->entry[i].name, "/boot/pconf")) {
			config_parse((char*) object_list->entry[i].base);
			break;
		}
	}

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

	// pinion (pagefault, zombie, etc.) interrupt vector page
	pinion_vector_page_vtable.on_reset = pinion_on_reset;
	interrupt_add_vector_page(0x0080, &pinion_vector_page_vtable);

	// IRQ interrupt vector page
	irq_vector_page_vtable.on_fire = irq_on_fire;
	irq_vector_page_vtable.on_reset = irq_on_reset;
	interrupt_add_vector_page(0x0100, &irq_vector_page_vtable);

	// initialize ACPI (for IRQ routing info)
	init_acpi();

	// allocate initial thread TCB and add to scheduler
	scheduler_add_tcb(tcb_new());

	// schedule first thread
	scheduler_schedule();

	// load kernel image
	load_kernel(object_list);
}
