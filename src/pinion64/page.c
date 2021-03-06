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

// XXX
// This file here is basically all hacked together, and will probably explode
// if there are out-of-memory errors anywhere (and doesn't handle updating
// large pages properly.) Fix it once everything seems to be working, please!
//
// This could also be rewritten in assembly for some extra speed, considering
// it is the core of all memory management.
// XXX

static uint64_t base = 0xFFFFFFFFC0000000ULL;

static uint64_t *pcx_root;

static uint8_t pcx_root_lock;

void pcx_init(void) {

	// allocate and initialize root paging context
	pcx_root = pge_alloc();
	for (int i = 0; i < 512; i++) {
		pcx_root[i] = 0;
	}

	for (int i = 256; i < 512; i++) {
		uint64_t *pt3 = pge_alloc();
		uint64_t frame = (uint64_t) pt3 - base;

		pcx_root[i] = frame | PF_PRES | PF_WRITE;
		for (int j = 0; j < 512; j++) {
			pt3[j] = 0;
		}
	}

	// initialize pinion mappings
	uint64_t *pinion_pt2 = pge_alloc();
	for (int i = 0; i < 256; i++) {
		pinion_pt2[i] = ((uint64_t) i << 21) | PF_VALID | PF_PRES | PF_WRITE | PF_LARGE | PF_GLOBL;
	}
	for (int i = 256; i < 512; i++) {
		pinion_pt2[i] = (((uint64_t) i << 21) + 0xC0000000) 
			| PF_VALID | PF_PRES | PF_WRITE | PF_LARGE | PF_GLOBL | PF_PCD;
	}

	uint64_t *pinion_pt3 = (void*) ((pcx_root[511] + base) & ~0xFFF);
	pinion_pt3[511] = (((uint64_t) pinion_pt2) - base) | PF_PRES | PF_WRITE;

	// switch to new root paging context
	pcx_switch(pcx_root);
}

void *pmm_vaddr(uint64_t paddr) {
	
	if (paddr < 0x20000000) {
		return (void*) (paddr + base);
	}
	else if (paddr > 0xE0000000 && paddr < 0x100000000) {
		return (void*) (paddr - 0xE0000000 + base + 0x20000000);
	}
	else {
		return NULL;
	}
}

uint64_t *pcx_get(uint32_t pcx_id) {

	if (pcx_id == 0) {
		return pcx_root;
	}

	return NULL;
}

uint64_t pcx_get_trans(uint64_t *pcx, uint64_t addr) {

	if (!pcx) pcx = pcx_root;

	uint64_t *pt = pcx;

	for (int i = 4; i > 0; i--) {
		uint64_t idx = (addr >> (12 + 9 * (i - 1))) & 0x1FF;

		if (!(pt[idx] & PF_PRES)) {
			mutex_release(&pcx_root_lock);
			return -1;
		}
		
		if (pt[idx] & PF_LARGE || i == 1) {
			mutex_release(&pcx_root_lock);
			return (pt[idx] &~ PF_BITS) + (addr & ((1 << (12 + 9 * (i - 1))) - 1));
		}

		pt = (void*) ((pt[idx] &~ PF_BITS) + base);
	}

	return -1;
}

uint64_t pcx_get_frame(uint64_t *pcx, uint64_t addr) {	
	return pcx_get_trans(pcx, addr & -0x1000);
}

uint64_t pcx_get_flags(uint64_t *pcx, uint64_t addr) {
	
	if (!pcx) pcx = pcx_root;

	uint64_t *pt = pcx;

	for (int i = 4; i > 0; i--) {
		uint64_t idx = (addr >> (12 + 9 * (i - 1))) & 0x1FF;

		if (!(pt[idx] & PF_PRES)) {
			mutex_release(&pcx_root_lock);
			return 0;
		}
		
		if (pt[idx] & PF_LARGE || i == 1) {
			mutex_release(&pcx_root_lock);
			return (pt[idx] & PF_BITS);
		}

		pt = (void*) ((pt[idx] &~ PF_BITS) + base);
	}

	return 0;
}

int pcx_set_frame(uint64_t *pcx, uint64_t addr, uint64_t size, uint64_t frame, uint64_t flags) {
	
	// reject unaligned requests
	if ((size & 0xFFF) || (addr & 0xFFF)) return 1;

	if (!pcx) pcx = pcx_root;

	uint64_t *pt = pcx;

	for (int i = 4; i > 1; i--) {
		uint64_t idx = (addr >> (12 + 9 * (i - 1))) & 0x1FF;

		if (!(pt[idx] & PF_PRES)) {

			uint64_t *newpt = pge_alloc();
			pt[idx] = ((uint64_t) newpt - 0xFFFFFFFFC0000000) | PF_VALID | PF_PRES | PF_WRITE | PF_USER;
			for (uint32_t j = 0; j < 512; j++) {
				newpt[j] = 0ULL;
			}
		}

		if (pt[idx] & PF_LARGE) {
			mutex_release(&pcx_root_lock);
			return 1;
		}

		pt = (void*) ((pt[idx] &~ PF_BITS) + base);
	}

	uint64_t idx = (addr >> 12) & 0x1FF;
	pt[idx] = frame | flags;

	return 0;
}
