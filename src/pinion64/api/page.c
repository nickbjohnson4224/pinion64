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

#include <stdbool.h>

#include "../include/pinion.h"
#include "../internal.h"

//////////////////////////////////////////////////////////////////////////////
// API function page_set
//
// Sets one or more contiguous page mappings in the current thread's active 
// page table.
//
// vbase - virtual base address (must be page-aligned)
// pbase - physical base address (must be page-aligned)
// flags - flags to set on page mappings
// size  - size in bytes of region to change the mapping of (must be a 
//   multiple of the page size)
//
// Returns true (nonzero) on success, false (zero) on error.
//
// After this call has completed successfully, the virtual to physical mapping
// for the specified region will be as follows:
//   vbase + i => pbase + i (for all 0 <= i < size)
// The page flags for all mappings in the specified region will be changed to
// the given page flags.
//
// If an error occurs, no page mappings will change.
//

// TODO LIST
//
// pcx_set_frame is not that reliable and will not handle out-of-memory errors
// properly. This needs to be fixed to guarantee atomicity of this call.
//
// This call does not actually guarantee atomicity, even though the docs claim
// it does. This will only be a problem if pinion runs out of memory (and 
// later when SMP is implemented) but should be fixed ASAP.
//
// When multiple pagetable support is added, this needs to reference the 
// current thread's pagetable instead of just the root one.

bool apilogic_page_set(__PINION_virtaddr vbase, __PINION_virtaddr pbase, 
	__PINION_pageflags flags, size_t size) {

	// reject unaligned requests
	if ((pbase & 0xFFF) || (vbase & 0xFFF) || (size & 0xFFF)) {
		return false;
	}

	// convert page flags
	uint64_t raw_flags = 0;

	if (flags & __PINION_PAGE_VALID) {
		raw_flags |= PF_VALID;
		
		if (flags & (__PINION_PAGE_READ | __PINION_PAGE_WRITE | __PINION_PAGE_EXEC)) {
			raw_flags |= PF_PRES;
			if (!(flags & __PINION_PAGE_READ)) raw_flags |= PF_NREAD;
			if (flags & __PINION_PAGE_WRITE)   raw_flags |= PF_WRITE;
			if (!(flags & __PINION_PAGE_EXEC)) raw_flags |= PF_NX;
			if (flags & __PINION_PAGE_USER)    raw_flags |= PF_USER;
			if (flags & __PINION_PAGE_WRITE_THROUGH) raw_flags |= PF_PWT;
			if (flags & __PINION_PAGE_CACHE_DISABLE) raw_flags |= PF_PCD;
		}

		if (flags & __PINION_PAGE_ACCESSED) raw_flags |= PF_ACCS;
		if (flags & __PINION_PAGE_DIRTY)    raw_flags |= PF_DIRTY;
	}

	// set pages
	for (size_t i = 0; i < size; i += 0x1000) {
		if (pcx_set_frame(NULL, vbase + i, 0x1000, pbase + i, flags)) {
			return false;
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////////
// API function page_get
//
// Gets the virtual to physical mapping and page flags for a given virtual
// address in the current thread's active page table.
//
// vaddr - virtual address
//
// Returns a structure containing the mapped-to physical address and the
// effective page flags. This call cannot fail.
//

// TODO LIST
//
// When multiple pagetable support is added, this needs to reference the 
// current thread's pagetable instead of just the root one.

struct __PINION_page_content apilogic_page_get(__PINION_virtaddr vaddr) {

	struct __PINION_page_content page = {
	
		// get virtual->physical translation
		pcx_get_trans(NULL, vaddr),

		// used for accumulating cooked page flags 
		0
	};

	// get page flags
	uint64_t raw_flags = pcx_get_flags(NULL, vaddr);

	// convert page flags
	if (raw_flags & PF_VALID) {
		page.flags |= __PINION_PAGE_VALID;

		if (raw_flags & PF_PRES) {
			if (!(raw_flags & PF_NREAD)) page.flags |= __PINION_PAGE_READ;
			if (raw_flags & PF_WRITE)    page.flags |= __PINION_PAGE_WRITE;
			if (!(raw_flags & PF_NX))    page.flags |= __PINION_PAGE_EXEC;
			if (raw_flags & PF_USER)     page.flags |= __PINION_PAGE_USER;
			if (raw_flags & PF_PWT)      page.flags |= __PINION_PAGE_WRITE_THROUGH;
			if (raw_flags & PF_PCD)      page.flags |= __PINION_PAGE_CACHE_DISABLE;
		}

		if (raw_flags & PF_ACCS)  page.flags |= __PINION_PAGE_ACCESSED;
		if (raw_flags & PF_DIRTY) page.flags |= __PINION_PAGE_DIRTY;
	}

	return page;
}
