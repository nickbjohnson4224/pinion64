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

__PINION_thread_id apilogic_thread_create(const struct __PINION_thread_state *proto) {

	struct tcb *new_tcb = tcb_new();

	if (!new_tcb) {
		return 0;
	}

	// TODO add flags to ignore (for speed)
	new_tcb->rax = proto->rax;
	new_tcb->rcx = proto->rcx;
	new_tcb->rbx = proto->rbx;
	new_tcb->rdx = proto->rdx;
	new_tcb->rdi = proto->rdi;
	new_tcb->rsi = proto->rsi;
	new_tcb->rbp = proto->rbp;
	new_tcb->r8  = proto->r8;
	new_tcb->r9  = proto->r9;
	new_tcb->r10 = proto->r10;
	new_tcb->r11 = proto->r11;
	new_tcb->r12 = proto->r12;
	new_tcb->r13 = proto->r13;
	new_tcb->r14 = proto->r14;
	new_tcb->r15 = proto->r15;

	// TODO load xstate
	
	// important stuff
	new_tcb->state = TCB_STATE_QUEUED;
	new_tcb->rip = proto->rip;
	new_tcb->rsp = proto->rsp;

	scheduler_add_tcb(new_tcb);

	return new_tcb->id;
}