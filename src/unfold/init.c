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
#include <stddef.h>

#include "log.h"

// multiboot header

#define MBOOT_FLAG_MEM     0x0001
#define MBOOT_FLAG_DEVICE  0x0002
#define MBOOT_FLAG_CMDLINE 0x0004
#define MBOOT_FLAG_MODS    0x0008
#define MBOOT_FLAG_SYMS    0x0030
#define MBOOT_FLAG_MMAP    0x0040
#define MBOOT_FLAG_DRIVES  0x0080
#define MBOOT_FLAG_NAME    0x0100
#define MBOOT_FLAG_APM     0x0200
#define MBOOT_FLAG_VBE     0x0400

struct multiboot_module {
	uint32_t mod_start;
	uint32_t mod_end;
	const char *string;
	uint32_t reserved;
};

struct multiboot_mmap {
	uint32_t size;
	uint64_t base_addr;
	uint64_t length;
	uint32_t type;
};

struct multiboot {
	uint32_t flags;

	uint32_t mem_lower;
	uint32_t mem_upper;

	uint32_t boot_device;

	const char *cmdline;

	uint32_t mods_count;
	const struct multiboot_module *mods_addr;

	uint32_t syms0;
	uint32_t syms1;
	uint32_t syms2;
	uint32_t syms3;

	uint32_t mmap_count;
	uint32_t mmap_addr;
};

// init

void init(const struct multiboot *mboot, uint32_t magic) {
	int pinion64_mod = -1;

	if (magic != 0x2BADB002) {
		// non-multiboot loader; panic
		log(ERROR, "invalid multiboot magic number: %x", magic);
		return;
	}

	log(INIT, "unfold (pinion loader) starting...");

	// read multiboot memory map
	if (mboot->flags & MBOOT_FLAG_MMAP) {
		struct multiboot_mmap *mmap = (void*) (mboot->mmap_addr);
		uint64_t totalmem = 0;

		for (uint32_t i = 0; i < mboot->mmap_count;) {

			uint64_t base = mmap->base_addr;
			uint32_t basel = base & 0xFFFFFFFFULL;
			uint32_t baseh = (base >> 32) & 0xFFFFFFFFULL;

			uint64_t limit = mmap->base_addr + mmap->length;
			uint32_t limitl = limit & 0xFFFFFFFFULL;
			uint32_t limith = (limit >> 32) & 0xFFFFFFFFULL;

			log(INFO, "memory region: [%x%x %x%x] type %d (%d KB)", baseh, basel,	
				limith, limitl, mmap->type, (uint32_t) (mmap->length >> 10));

			if (mmap->type == 1) {
				totalmem += mmap->length;
			}

			i += mmap->size + 4;
			mmap = (void*) ((uint32_t) mmap + mmap->size + 4);
		}

		log(INIT, "%d KB of memory available", totalmem >> 10);
	}
	else {
		log(ERROR, "no memory map found");
		return;
	}

	// locate pinion64 module
	if (mboot->flags & MBOOT_FLAG_MODS) {

		for (uint32_t i = 0; i < mboot->mods_count; i++) {
			log(INFO, "module %d: %s at %x (%d KB)", i, 
				mboot->mods_addr->string, mboot->mods_addr->mod_start, 
				(mboot->mods_addr->mod_end - mboot->mods_addr->mod_start) >> 10);

			const char *string = mboot->mods_addr->string;
			const char *pinion64 = "pinion64";

			int j;
			for (j = 0; string[j]; j++);
			for (; string[j] != '/' && j >= 0; j--);
			j++;

			int k;
			for (k = 0; pinion64[k] && string[j+k]; k++) {
				if (pinion64[k] != string[j+k]) break;
			}

			if (k == 8) {
				log(INIT, "pinion64 found in module %d", i);
				pinion64_mod = i;
			}
		}

		log(INFO, "%d module(s) loaded", mboot->mods_count);
	}
	else {
		log(ERROR, "no modules found");
		return;
	}

	if (pinion64_mod == -1) {
		log(ERROR, "no pinion64 module found");
		return;
	}

	// check pinion64 module
	
}
