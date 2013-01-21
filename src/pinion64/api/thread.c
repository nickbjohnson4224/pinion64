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

#include "../include/pinion.h"
#include "../internal.h"

void apilogic_thread_yield(void) {

	// actually implemented; see api.s
	return;
}

__PINION_thread_id apilogic_thread_create(const struct __PINION_thread_state *proto) {

	struct tcb *new_tcb = tcb_new();

	if (!new_tcb) {
		return 0;
	}

	new_tcb->state = TCB_STATE_QUEUED;
	new_tcb->rip = proto->rip;
	new_tcb->rsp = proto->rsp;

	scheduler_add_tcb(new_tcb);

	return new_tcb->id;
}

bool apilogic_thread_pause(__PINION_thread_id thread) {
	struct ccb *ccb = ccb_get_self();

	if (thread == (__PINION_thread_id) __PINION_THREAD_ID_SELF) {
		thread = ccb->active_tcb->id;
	}

	struct tcb *tcb = tcb_get(thread);

	if (!tcb) {
		return false;
	}

	switch (tcb->state) {
	case TCB_STATE_ZOMBIE:

		// zombies can only be reaped
		return false;
		break;

	case TCB_STATE_SUSPENDED:
	case TCB_STATE_SUSPENDEDWAITING:

		// already suspended
		break;

	case TCB_STATE_WAITING:

		// make suspended waiting (still in waitlist)
		tcb->state = TCB_STATE_SUSPENDEDWAITING;
		break;

	case TCB_STATE_QUEUED:

		// unqueue
		scheduler_rem_tcb(tcb);
		tcb->state = TCB_STATE_SUSPENDED;
		break;

	case TCB_STATE_RUNNING:

		// will be taken care of by post-yield
		tcb->state = TCB_STATE_SUSPENDED;
		break;

	default:

		// should never happen!
		return false;

	}

	return true;
}

bool apilogic_thread_resume(__PINION_thread_id thread) {
	struct ccb *ccb = ccb_get_self();

	if (thread == (__PINION_thread_id) __PINION_THREAD_ID_SELF) {
		thread = ccb->active_tcb->id;
	}

	struct tcb *tcb = tcb_get(thread);

	if (!tcb) {
		return false;
	}

	switch (tcb->state) {
	case TCB_STATE_SUSPENDED:

		// queue
		scheduler_add_tcb(tcb);
		tcb->state = TCB_STATE_QUEUED;
		break;

	case TCB_STATE_SUSPENDEDWAITING:

		// put back in waiting state
		tcb->state = TCB_STATE_WAITING;
		break;

	default:

		// not suspended!
		return false;

	}

	return true;
}

void apilogic_thread_exit(void) {
	struct ccb *ccb = ccb_get_self();

	ccb->active_tcb->state = TCB_STATE_ZOMBIE;

	// TODO add to zombie queue
}

bool apilogic_thread_set_tap(__PINION_tap_index index, __PINION_interrupt_vector vec) {
	struct tcb *tcb = ccb_get_self()->active_tcb;

	if (index >= 4) {
		// tap index too large
		return false;
	}

	if (vec & 0xC000) {
		// vector invalid
		return false;
	}

	// remove old tap
	if (tcb->tap[index] & 0x3FFF) {
		// remove old interrupt route
		interrupt_vector_remove(tcb, tcb->tap[index]);
	}

	// insert new tap
	tcb->tap[index] = vec;

	// add new interrupt route
	interrupt_vector_add(tcb, tcb->tap[index]);
	interrupt_vector_reset(tcb->tap[index]);

	return true;
}

__PINION_interrupt_vector apilogic_thread_get_tap(__PINION_tap_index index) {
	struct tcb *tcb = ccb_get_self()->active_tcb;

	if (index >= 4) {
		// tap index too large
		return 0x0000;
	}

	return tcb->tap[index];	
}

void apilogic_thread_reset(__PINION_interrupt_vector vec) {
	struct tcb *tcb = ccb_get_self()->active_tcb;
	
	__PINION_tap_index i;
	for (i = 0; i < 4; i++) {
		if ((tcb->tap[i] &~ 0xC000) == vec) {
			break;
		}
	}

	if (i == 4) {
		// untapped vector
		return;
	}

	// reset local tap state
	tcb->tap[i] &= ~0x8000;

	// TODO reset global vector state
	interrupt_vector_reset(vec);
}

__PINION_interrupt_vector apilogic_thread_wait(void) {
	struct tcb *tcb = ccb_get_self()->active_tcb;

	for (__PINION_tap_index i = 0; i < 4; i++) {
		if (tcb->tap[i] & 0x8000) {
			// tap already registered an interrupt, don't actually wait
			return tcb->tap[i] &~ 0x8000;
		}
	}

	tcb->state = TCB_STATE_WAITING;
	return 0; // will be patched to the correct vector
}
