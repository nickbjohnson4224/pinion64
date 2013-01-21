// Copyright (C) 2013 Nick Johnson <nickbjohnson4224 at gmail.com>
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

#define STATE_FLAG_FIRED  1

struct interrupt_route {
	struct tcb *tap[4];
};

struct interrupt_vector_page {
	struct interrupt_route *route;
	const struct interrupt_vector_page_vtable *vtable;
};

static struct interrupt_vector_page interrupt_vector_page_table[128];

void interrupt_add_vector_page(uint16_t vec_base, const struct interrupt_vector_page_vtable *vtable) {

	interrupt_vector_page_table[vec_base >> 7].route = pge_alloc();
	interrupt_vector_page_table[vec_base >> 7].vtable = vtable;

	for (size_t i = 0; i < 128; i++) {
		for (size_t j = 0; j < 4; j++) {
			interrupt_vector_page_table[vec_base >> 7].route[i].tap[j] = NULL;
		}
	}
}

void interrupt_add_route(struct tcb *tcb, uint16_t vec) {

	if (interrupt_vector_page_table[vec >> 7].route) {
		struct interrupt_vector_page *page = 
			&interrupt_vector_page_table[vec >> 7];

		// install route
		for (size_t i = 0; i < 4; i++) {
			if (!page->route[vec & 0x7F].tap[i]) {
				page->route[vec & 0x7F].tap[i] = tcb;
				return;
			}
		}

		// route could not be installed
		log(ERROR, "too many routes for vector %x", vec);
	}
}

void interrupt_remove_route(struct tcb *tcb, uint16_t vec) {

	if (interrupt_vector_page_table[vec >> 7].route) {
		struct interrupt_vector_page *page =
			&interrupt_vector_page_table[vec >> 7];

		for (size_t i = 0; i < 4; i++) {
			if (page->route[vec & 0x7F].tap[i] == tcb) {
				page->route[vec & 0x7F].tap[i] = NULL;
				return;
			}
		}
	}
}

void interrupt_vector_reset(uint16_t vec) {
	
	if (interrupt_vector_page_table[vec >> 7].route) {
		struct interrupt_vector_page *page =
			&interrupt_vector_page_table[vec >> 7];

		if (page->vtable && page->vtable->on_reset) {
			page->vtable->on_reset(vec);
		}
	}
}

void interrupt_vector_fire(uint16_t vec) {

	if (interrupt_vector_page_table[vec >> 7].route) {
		struct interrupt_vector_page *page =
			&interrupt_vector_page_table[vec >> 7];

		if (page->vtable && page->vtable->on_fire) {
			page->vtable->on_fire(vec);
		}

		// notify taps
		for (size_t i = 0; i < 4; i++) {
			struct tcb *tcb = page->route[vec & 0x7F].tap[i];
			if (!tcb) continue;

			for (size_t j = 0; j < 4; j++) {
				if ((tcb->tap[j] & 0x3FFF) != vec) continue;

				// set interrupt fired flag
				tcb->tap[j] |= 0x8000;

				switch (tcb->state) {
				case TCB_STATE_WAITING:

					scheduler_add_tcb(tcb);
					tcb->state = TCB_STATE_QUEUED;
					tcb->rax = vec;
					break;

				case TCB_STATE_SUSPENDEDWAITING:

					tcb->state = TCB_STATE_SUSPENDED;
					tcb->rax = vec;
					break;
				}

				break;
			}
		}
	}
}
