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

#ifndef __UNFOLD_MULTIBOOT_H
#define __UNFOLD_MULTIBOOT_H

#include <stdint.h>

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

struct multiboot_header {
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

void init_multiboot(const struct multiboot_header *mboot);

#endif//__UNFOLD_MULTIBOOT_H
