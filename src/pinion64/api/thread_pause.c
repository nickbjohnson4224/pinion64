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
