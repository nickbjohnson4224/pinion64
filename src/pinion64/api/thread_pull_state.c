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

bool apilogic_thread_pull_state(__PINION_thread_id thread, struct __PINION_thread_state *state) {
	
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

	// copy state
	state->rax = tcb->rax;
	state->rcx = tcb->rcx;
	state->rbx = tcb->rbx;
	state->rdx = tcb->rdx;
	state->rdi = tcb->rdi;
	state->rsi = tcb->rsi;
	state->rbp = tcb->rbp;
	state->r8  = tcb->r8;
	state->r9  = tcb->r9;
	state->r10 = tcb->r10;
	state->r11 = tcb->r11;
	state->r12 = tcb->r12;
	state->r13 = tcb->r13;
	state->r14 = tcb->r14;
	state->r15 = tcb->r15;

	state->rip = tcb->rip;
	state->rsp = tcb->rsp;

	state->gs = tcb->gs;
	state->fs = 0;
	state->cs = tcb->cs;
	state->ss = tcb->ss;
	state->ds = tcb->ss;

	// TODO copy xstate
	
	return true;
}
