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

static struct tcb *schedule_head;
static struct tcb *schedule_tail;

static void scheduler_push_tcb(struct tcb *tcb) {
	
	tcb->next = NULL;
	if (schedule_tail) {
		schedule_tail->next = tcb;
	}
	else {
		schedule_head = tcb;
	}
	schedule_tail = tcb;
}

static struct tcb *scheduler_pull_tcb(void) {
	struct tcb *tcb;

	tcb = schedule_head;

	if (schedule_head) {
		schedule_head = schedule_head->next;
	}
	if (schedule_tail == tcb) {
		schedule_tail = NULL;
	}

	return tcb;
}

void scheduler_add_tcb(struct tcb *tcb) {

	scheduler_push_tcb(tcb);
}

int scheduler_rem_tcb(struct tcb *tcb) {

	if (schedule_head == tcb) {
		schedule_head = schedule_head->next;
		if (schedule_tail == tcb) {
			schedule_tail = NULL;
		}

		return 0;
	}

	for (struct tcb *t = schedule_head; t; t = t->next) {
		if (t->next == tcb) {
			t->next = tcb->next;
			if (schedule_tail == tcb) {
				schedule_tail = t;
			}
			return 0;
		}
	}

	return 1;
}

void scheduler_schedule(void) {

	// unload current TCB
	struct tcb *old_tcb = ccb_unload_tcb();

	if (old_tcb) {
		old_tcb->state = TCB_STATE_QUEUED;
		scheduler_push_tcb(old_tcb);
	}

	struct tcb *new_tcb = scheduler_pull_tcb();

	if (new_tcb) {
		new_tcb->state = TCB_STATE_RUNNING;
		ccb_load_tcb(new_tcb);
	}
	else {
		// TODO idle
	}
}
