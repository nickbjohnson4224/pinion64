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

struct pmm_map_entry {
	uint64_t base;
	uint64_t size;
};

struct pmm_map {
	uint64_t count;
	struct pmm_map_entry entry[];
};

static uint64_t pmm_map_cutoff;

static uint64_t pmm_map_low_space[7];
static uint64_t pmm_map_high_space[17];

static struct pmm_map * const pmm_map_low  = (void*) pmm_map_low_space;
static struct pmm_map * const pmm_map_high = (void*) pmm_map_high_space;

struct unfold_mmap {
	uint64_t total;
	uint64_t count;
	struct pmm_map_entry entry[];
};

static uint8_t  pmm_mutex = 0;
uint64_t pmm_base;
static uint64_t pmm_alloc_base;
static uint64_t pmm_alloc_limit;

void *pmm_alloc(void) {
	uint64_t addr;
	
	mutex_acquire(&pmm_mutex);

	if (pmm_alloc_base == pmm_alloc_limit) {
		addr = 0;
	}
	else {
		addr = pmm_alloc_base + pmm_base;
		pmm_alloc_base += 0x1000;
	}

	mutex_release(&pmm_mutex);

	return (void*) addr;
}

void pmm_init(void *_mmap) {
	struct unfold_mmap *mmap = _mmap;
	uint32_t i, j = 0;
	uint64_t base;
	uint64_t limit;

	extern void get_pinion_range(uint64_t *, uint64_t*);
	get_pinion_range(&base, &limit);
	pmm_base = base - 0x200000;

	// calculate cutoff value based on total RAM
	// cutoff = total / 256 rounded up to 8MB
	pmm_map_cutoff = -(-(mmap->total / 256) & (-1 << 23));

	pmm_alloc_base  = limit - pmm_base;
	pmm_alloc_limit = pmm_map_cutoff;

	log(INFO, "%d fully-free pinion frames", (pmm_map_cutoff - limit + base - 0x200000) >> 12);

	// fill in low memory map
	for (i = 0; i < mmap->count; i++) {

		if (i >= 6) {
			pmm_map_low->count = i;
			j = i;
			i = 0;
			break;
		}

		if (mmap->entry[i].base < pmm_map_cutoff) {
			pmm_map_low->entry[i].base = mmap->entry[i].base;
			if (mmap->entry[i].base + mmap->entry[i].size <= pmm_map_cutoff) {
				pmm_map_low->entry[i].size = mmap->entry[i].size;
			}
			else {
				// take the low half of an entry
				pmm_map_low->entry[i].size = pmm_map_cutoff - mmap->entry[i].base;
				pmm_map_high->entry[0].base = pmm_map_cutoff;
				pmm_map_high->entry[0].size = mmap->entry[i].size + mmap->entry[i].base - pmm_map_cutoff;
				pmm_map_low->count = i + 1;
				j = i;
				i = 1;
				break;
			}
		}
		else {
			// cut off here
			pmm_map_low->count = i;
			j = i;
			i = 0;
			break;
		}
	}

	// fill in high memory map
	for (; i + j < mmap->count && i < 16; i++) {
		pmm_map_high->entry[i].base = mmap->entry[i+j].base;
		pmm_map_high->entry[i].size = mmap->entry[i+j].size;
	}
	pmm_map_high->count = i;

	// list memory map contents
	log(INFO, "listing low memory map:");
	for (i = 0; i < pmm_map_low->count; i++) {
		log(INFO, "[%p %p]", pmm_map_low->entry[i].base,
			pmm_map_low->entry[i].base + pmm_map_low->entry[i].size);
	}

	log(INFO, "listing high memory map:");
	for (i = 0; i < pmm_map_high->count; i++) {
		log(INFO, "[%p %p]", pmm_map_high->entry[i].base,
			pmm_map_high->entry[i].base + pmm_map_high->entry[i].size);
	}
}

uint64_t pmm_get_pinion_load_addr(void) {
	return pmm_base + 0x200000;
}
