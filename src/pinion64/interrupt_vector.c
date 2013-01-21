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

struct interrupt_route irq_route[128];

void interrupt_vector_add(struct tcb *tcb, uint16_t vec) {
	
	if ((vec & 0xFF80) == 0x0100) {
		// IRQ route

		// install route
		for (size_t i = 0; i < 4; i++) {
			if (irq_route[vec & 0x7F].tap[i] == NULL) {
				irq_route[vec & 0x7F].tap[i] = tcb;
				break;
			}
		}
	}
}

void interrupt_vector_remove(struct tcb *tcb, uint16_t vec) {

	if ((vec & 0xFF80) == 0x0100) {
		// IRQ route

		// remove route
		for (size_t i = 0; i < 4; i++) {
			if (irq_route[vec & 0x7F].tap[i] == tcb) {
				irq_route[vec & 0x7F].tap[i] = NULL;
				break;
			}
		}
	}
}

void interrupt_vector_reset(uint16_t vec) {
	
	if ((vec & 0xFF80) == 0x0100) {
		// IRQ vector

		// TODO enable IRQ
	}
}

void interrupt_vector_fire(uint16_t vec) {

	if ((vec & 0xFF80) == 0x0100) {
		// IRQ vector

		// TODO disable IRQ

		// notify taps
		for (size_t i = 0; i < 4; i++) {
			if (irq_route[vec & 0x7F].tap[i]) {
				struct tcb *tcb = irq_route[vec & 0x7F].tap[i];

				for (size_t i = 0; i < 4; i++) {
					if ((tcb->tap[i] & 0x3FFF) == vec) {
						tcb->tap[i] |= 0x8000;
						if (tcb->state == TCB_STATE_WAITING) {
							scheduler_add_tcb(tcb);
							tcb->state = TCB_STATE_QUEUED;
							tcb->rax = vec;
							scheduler_schedule();
						}
						else if (tcb->state == TCB_STATE_SUSPENDEDWAITING) {
							tcb->state = TCB_STATE_SUSPENDED;
							tcb->rax = vec;
						}
						break;
					}
				}
			}
		}
	}
}
