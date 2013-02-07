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
// Interrupts
//

#define __PINION_INTERRUPT_VECTOR_NULL      0x0000
#define __PINION_INTERRUPT_VECTOR_IRQ(n)    (0x0100 + (n))
#define __PINION_INTERRUPT_VECTOR_PAGEFAULT 0x0080
#define __PINION_INTERRUPT_VECTOR_MISCFAULT 0x0081
#define __PINION_INTERRUPT_VECTOR_ZOMBIE    0x0082

typedef uint16_t __PINION_interrupt_vector;

#define __PINION_TAP_COUNT 4

typedef uint8_t __PINION_tap_index;

bool 
__PINION_interrupt_set_tap(
	__PINION_tap_index index, 
	__PINION_interrupt_vector vec);

__PINION_interrupt_vector 
__PINION_interrupt_get_tap(
	__PINION_tap_index index);

void 
__PINION_interrupt_reset(
	__PINION_interrupt_vector vec);

__PINION_interrupt_vector
	__PINION_interrupt_wait(void);

//
// Threading
//

#define __PINION_THREAD_ID_SELF -1

typedef uint32_t __PINION_thread_id;

struct __PINION_thread_state {

	// interrupt taps
	__PINION_interrupt_vector tap[__PINION_TAP_COUNT];
	uint64_t tap_state;

	// fault information (readonly)
	uint64_t fault_type;
	uint64_t fault_address;

	// register states

	// general-purpose registers
	uint64_t rax;
	uint64_t rcx;
	uint64_t rbx;
	uint64_t rdx;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rsp;
	uint64_t rbp;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;

	// instruction pointer
	uint64_t rip;

	// segment registers (readonly)
	uint64_t fs;
	uint64_t gs;
	uint64_t cs;
	uint64_t ss;
	uint64_t ds;

	// extended (SSE/AVX) registers
	// TODO figure out internal structure
	uint8_t xstate[512];

} __attribute__((packed));

__PINION_thread_id __PINION_thread_create(const struct __PINION_thread_state *state);
void __PINION_thread_yield(void);
bool __PINION_thread_pause (__PINION_thread_id thread);
bool __PINION_thread_pull_state(__PINION_thread_id thread, struct __PINION_thread_state *state);
bool __PINION_thread_push_state(__PINION_thread_id thread, const struct __PINION_thread_state *state);
bool __PINION_thread_resume(__PINION_thread_id thread);
void __PINION_thread_exit(void);
bool __PINION_thread_reap(__PINION_thread_id thread);

//
// Fault handling
//

__PINION_thread_id __PINION_thread_get_pagefault(void);
__PINION_thread_id __PINION_thread_get_miscfault(void);
__PINION_thread_id __PINION_thread_get_zombie(void);

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

#endif//__PINION_H
