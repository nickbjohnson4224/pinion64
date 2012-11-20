// Copyright (C) 2012 Nick Johnson <nickbjohnson4224 at gmail.com>
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

// maximum number of supported TCBs
// TODO export to configuration header
#define TCB_LIMIT 65536

static struct tcb ** tcb_table[TCB_LIMIT / 512];
static uint8_t tcb_table_lock;

// tcb_new - allocate a new thread control block
//
// Returns a pointer to a new, initialized TCB on success, NULL on error.
// The TCB will have a locked mutex, and will be in state READY.
// This function can fail if there is no memory available for new TCBs or
// if there are no free TCB table slots.
// 
struct tcb *tcb_new(void) {

	// try to allocate TCB
	struct tcb *tcb = tcb_alloc();

	if (!tcb) {
		
		// allocation failed, return NULL
		return NULL;
	}

	mutex_acquire(&tcb_table_lock);

	// search for empty TCB slot
	for (uint32_t i = 0; i < TCB_LIMIT; i++) {

		if (!tcb_table[i / 512]) {

			// try to allocate new TCB subtable
			// XXX should we ever allocate while holding the TCB table lock?
			struct tcb **subtab = pge_alloc();

			if (!subtab) {

				// allocation failed, return NULL
				mutex_release(&tcb_table_lock);
				tcb_free(tcb);
				return NULL;
			}

			// initialize subtable
			tcb_table[i / 512] = subtab;
			for (int j = 0; j < 512; j++) {
				tcb_table[i / 512][j] = NULL;
			}
		}

		if (!tcb_table[i / 512][i % 512]) {
			// empty TCB slot found

			// set entry in TCB table
			tcb_table[i / 512][i % 512] = tcb;

			// initialize TCB
			tcb->mutex = 1;
			tcb->state = TSTATE_RE;
			tcb->id    = i;

			// return new TCB
			mutex_release(&tcb_table_lock);
			return tcb;
		}

	}

	// no free slots, free TCB and return NULL
	mutex_release(&tcb_table_lock);
	tcb_free(tcb);
	return NULL;
}

// tcb_del - delete a thread control block
//
// The TCB must have its mutex locked and be valid.
//
void tcb_del(struct tcb *tcb) {

	// check that TCB is non-null
	if (!tcb) {
		
		log(ERROR, "tcb_del - TCB null");
		return;
	}

	// check that TCB is valid
	if (!(tcb->state & TSFLAG_VALID) || tcb->id >= TCB_LIMIT) {

		log(ERROR, "tcb_del - TCB not valid");
		return;
	}

	// make sure TCB lock is acquired
	if (!tcb->mutex) {
		
		log(ERROR, "tcb_del - TCB lock not pre-acquired");
		return;
	}
	
	mutex_acquire(&tcb_table_lock);
	
	// check that subtable exists
	if (!(tcb_table[tcb->id / 512])) {

		log(ERROR, "tcb_del - TCB does not have subtable (must be invalid)");
		mutex_release(&tcb_table_lock);
		return;
	}

	// remove from TCB table
	tcb_table[tcb->id / 512][tcb->id % 512] = NULL;

	mutex_release(&tcb_table_lock);

	// free TCB structure
	tcb_free(tcb);
}

// tcb_get - get a TCB by thread id
//
// Returns a pointer to the TCB on success, NULL on error.
//
struct tcb *tcb_get(uint32_t thread_id) {

	if (thread_id >= TCB_LIMIT) {

		// ID too large, return NULL
		return NULL;
	}

	mutex_acquire(&tcb_table_lock);

	if (!(tcb_table[thread_id / 512])) {

		// no subtable, return NULL
		mutex_release(&tcb_table_lock);
		return NULL;
	}

	// get TCB from table
	struct tcb *tcb = tcb_table[thread_id / 512][thread_id % 512];

	// return TCB
	mutex_release(&tcb_table_lock);
	return tcb;
}
