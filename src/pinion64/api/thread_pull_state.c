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

bool apilogic_thread_pull_state(
		__PINION_thread_id thread, 
		__PINION_thread_state_class state_class, 
		struct __PINION_thread_state *state) {
	
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

	if (state_class & __PINION_THREAD_STATE_CLASS_REGS_IP) {
		state->rip = tcb->rip;
	}
	if (state_class & __PINION_THREAD_STATE_CLASS_REGS_SP) {
		state->rsp = tcb->rsp;
	}
	if (state_class & __PINION_THREAD_STATE_CLASS_REGS_CALLEE) {
		state->rbx = tcb->rbx;
		state->rbp = tcb->rbp;
		state->r12 = tcb->r12;
		state->r13 = tcb->r13;
		state->r14 = tcb->r14;
		state->r15 = tcb->r15;
	}
	if (state_class & __PINION_THREAD_STATE_CLASS_REGS_CALLER) {
		state->rax = tcb->rax;
		state->rcx = tcb->rcx;
		state->rdx = tcb->rdx;
		state->rdi = tcb->rdi;
		state->rsi = tcb->rsi;
		state->r8  = tcb->r8;
		state->r9  = tcb->r9;
		state->r10 = tcb->r10;
		state->r11 = tcb->r11;
	}
	if (state_class & __PINION_THREAD_STATE_CLASS_REGS_SYSTEM) {
		state->gs = tcb->gs;
		state->fs = 0;
		state->cs = tcb->cs;
		state->ss = tcb->ss;
		state->ds = tcb->ss;
	}
	
	return true;
}
