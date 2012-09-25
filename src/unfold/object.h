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

#ifndef __UNFOLD_OBJECT_H
#define __UNFOLD_OBJECT_H

#include <stdint.h>
#include <stddef.h>

#define UNFOLD_OBJECT_UNKNOWN 0xFFFFFFFF
#define UNFOLD_OBJECT_PINION  0x00000001

struct unfold_object {
	uint64_t base;
	uint32_t size;
	uint32_t type;
} __attribute__((packed));

struct unfold_object_list {
	uint32_t count;
	uint32_t __padding;
	struct unfold_object entry[];
} __attribute__((packed));

extern struct unfold_object_list *object_list;
extern struct unfold_object *pinion_image;

int init_object_list();

#endif//__UNFOLD_OBJECT_H
