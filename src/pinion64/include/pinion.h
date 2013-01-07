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

#ifndef __PINION_H
#define __PINION_H

#include <stdint.h>
#include <stddef.h>

//
// API identification
//

extern const int pinion_api_major;
extern const int pinion_api_minor;
extern const char *pinion_implementation;

//
// Threading
//

struct pinion_thread_state;

void pinion_thread_yield(void);
void pinion_thread_spawn(const struct pinion_thread_state *state);
void pinion_thread_exit(void);

//
// Paging Contexts
//

//
// Object Manager
//

struct pinion_object {
	void *base;
	uint32_t size;
	uint32_t type;
	char name[48];
} __attribute__((packed));

struct pinion_object_list {
	uint32_t count;
	uint32_t reserved;
	struct pinion_object object[];
} __attribute__((packed));

const struct pinion_object_list *pinion_get_object_list(void);
const struct pinion_object *pinion_get_object(const char *name);
// future: int pinion_add_object(const struct pinion_object *object);
// future: void pinion_reconfigure(const char *config_object);

//
// Tweakable Extensions
//

int pinion_tweak(const char *feature, const char *field, const char *value);

#endif//__PINION_H
