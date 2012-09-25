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

#include "multiboot.h"
#include "object.h"
#include "mmap.h"
#include "log.h"

// TODO -- enough dynamic memory allocation to make this nonstatic
static uint8_t mboot_static_mmap[256];
static uint8_t mboot_static_objl[256];

void init_multiboot(const struct multiboot_header *mboot) {

	if (mboot->flags & MBOOT_FLAG_MMAP) {
		struct multiboot_mmap *m_mmap = (void*) (mboot->mmap_addr);

		// has multiboot memory map
		mmap = (void*) mboot_static_mmap;
		mmap->count = 0;
		mmap->total = 0;

		for (uint32_t i = 0; i < mboot->mmap_count;) {

			if (m_mmap->type == 1) {
				mmap->total += m_mmap->length;
				mmap->entry[mmap->count].base = m_mmap->base_addr;
				mmap->entry[mmap->count].size = m_mmap->length;
				mmap->count++;
			}

			i += m_mmap->size + 4;
			m_mmap = (void*) ((uint32_t) m_mmap + m_mmap->size + 4);
		}
	}

	if (mboot->flags & MBOOT_FLAG_MODS) {
		
		// has multiboot module list
		object_list = (void*) mboot_static_objl;
		object_list->count = mboot->mods_count;

		for (uint32_t i = 0; i < mboot->mods_count; i++) {

			// add to object list
			object_list->entry[i].base = mboot->mods_addr->mod_start;
			object_list->entry[i].size = mboot->mods_addr->mod_end - 
				mboot->mods_addr->mod_start;
			object_list->entry[i].type = UNFOLD_OBJECT_UNKNOWN;

			// check for pinion64 module
			// TODO -- less crappy parser
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
				object_list->entry[i].type = UNFOLD_OBJECT_PINION;
			}
		}
	}
}
