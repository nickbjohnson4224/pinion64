/*
 * Copyright (C) 2012 Nick Johnson <nickbjohnson4224 at gmail.com>
 * 
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdint.h>

#include "mmap.h"
#include "log.h"

struct unfold_mmap *mmap = NULL;

int init_mmap(void) {

	// use memory map generated by bootloader information
	if (!mmap) {
		return 1;
	}

	// calculate total size
	mmap->total = 0ULL;
	for (size_t i = 0; i < mmap->count; i++) {
		mmap->total += mmap->entry[i].size;
	}

	// print out memory map
	for (size_t i = 0; i < mmap->count; i++) {
		uint32_t basel = mmap->entry[i].base;
		uint32_t baseh = mmap->entry[i].base >> 32;
		uint32_t limitl = mmap->entry[i].base + mmap->entry[i].size;
		uint32_t limith = (mmap->entry[i].base + mmap->entry[i].size) >> 32;
			
		log(INFO, "memory: [%x%x %x%x] (%d KB)", baseh, basel, limith, limitl, 
			(uint32_t) (mmap->entry[i].size >> 10));
	}

	// print out total memory
	if (mmap->total < (1ULL << 24)) {
		log(INFO, "%d KB total memory available", mmap->total >> 10);
	}
	else if (mmap->total < (1ULL << 34)) {
		log(INFO, "%d MB total memory available", mmap->total >> 20);
	}
	else {
		log(INFO, "%d GB total memory available", mmap->total >> 30);
	}

	return 0;
}
