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
#include <stdbool.h>

//
// API identification
//

extern const int __PINION_api_major;
extern const int __PINION_api_minor;
extern const char *__PINION_implementation;

//
// Threading
//

struct __PINION_thread_state;

void __PINION_thread_yield(void);
void __PINION_thread_spawn(const struct __PINION_thread_state *state);
void __PINION_thread_exit(void);

//
// Paging
//

#define __PINION_PAGE_VALID         0x0001

#define __PINION_PAGE_READ          0x0010
#define __PINION_PAGE_WRITE         0x0020
#define __PINION_PAGE_EXEC          0x0040
#define __PINION_PAGE_USER          0x0080

#define __PINION_PAGE_ACCESSED      0x0100
#define __PINION_PAGE_DIRTY         0x0200

#define __PINION_PAGE_CACHE_DISABLE 0x1000
#define __PINION_PAGE_WRITE_THROUGH 0x2000

typedef uint8_t  __PINION_pagetable_id;
typedef uint64_t __PINION_virtaddr;
typedef uint64_t __PINION_physaddr;
typedef uint64_t __PINION_pageflags;

bool __PINION_page_set(__PINION_virtaddr virtual_base, 
	__PINION_physaddr physical_base,
	__PINION_pageflags page_flags, 
	size_t length);

struct __PINION_page_content {
	__PINION_physaddr  paddr;
	__PINION_pageflags flags;
} __attribute__((packed));

struct __PINION_page_content __PINION_page_get(
	__PINION_virtaddr virtual_base);

//
// Page Tables
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

const struct pinion_object_list *__PINION_get_object_list(void);
const struct pinion_object *__PINION_get_object(const char *name);

#endif//__PINION_H
