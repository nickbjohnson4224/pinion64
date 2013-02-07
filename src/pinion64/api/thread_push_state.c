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

bool apilogic_thread_push_state(__PINION_thread_id thread, const struct __PINION_thread_state *state) {
	
	// TODO 
	// check if *state is properly backed by memory.
	// we could crash pinion if bad pointers are passed.

	if (!state) {
		return false;
	}

	struct tcb *tcb = tcb_get(thread);

	if (!tcb) {
		return false;
	}

	if (tcb->state != TCB_STATE_SUSPENDED && tcb->state != TCB_STATE_SUSPENDEDWAITING) {
		return false;
	}

	// load state
	tcb->rax = state->rax;
	tcb->rcx = state->rcx;
	tcb->rbx = state->rbx;
	tcb->rdx = state->rdx;
	tcb->rdi = state->rdi;
	tcb->rsi = state->rsi;
	tcb->rbp = state->rbp;
	tcb->r8  = state->r8;
	tcb->r9  = state->r9;
	tcb->r10 = state->r10;
	tcb->r11 = state->r11;
	tcb->r12 = state->r12;
	tcb->r13 = state->r13;
	tcb->r14 = state->r14;
	tcb->r15 = state->r15;

	tcb->rip = state->rip;
	tcb->rsp = state->rsp;

	// TODO load xstate
	
	return true;
}
