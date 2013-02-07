// Copyright (C) 2012-2013 Nick Johnson <nickbjohnson4224 at gmail.com>
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

static int strcmp(const char *a, const char *b) {
	
	while (*a && *b) {
		if (*(a++) != *(b++)) return 1;
	}

	return 0;
}

#define APICATCH(NAME) \
do {\
	extern void apientry_##NAME(void);\
	\
	if (!strcmp(symbol, "__PINION_" #NAME)) {\
		return pinion_load_addr + (uint64_t) apientry_##NAME;\
	}\
} while (0) 

extern void pinion_thread_yield(void);

uint64_t api_resolve(const char *symbol) {
	static uint64_t pinion_load_addr = 0;
	
	if (!pinion_load_addr) pinion_load_addr = pmm_get_pinion_load_addr();

	APICATCH(thread_yield);
	APICATCH(thread_create);
	APICATCH(thread_pause);
	APICATCH(thread_resume);
	APICATCH(thread_exit);

	APICATCH(interrupt_set_tap);
	APICATCH(interrupt_get_tap);
	APICATCH(interrupt_reset);
	APICATCH(interrupt_wait);

	APICATCH(thread_get_pagefault);
	APICATCH(thread_get_miscfault);
	APICATCH(thread_get_zombie);

	APICATCH(page_set);
	APICATCH(page_get);

	return 0;
}
