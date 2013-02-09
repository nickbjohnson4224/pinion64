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

#ifndef __PINION_H
#define __PINION_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

//////////////////////////////////////////////////////////////////////////////
// Threading
//

//
// Thread ID Numbers
//
// Every thread is given an ID number that can be used as a handle to refer 
// to it in pinion API calls. This number is unique at any one point in time,
// but ID numbers may be recycled after threads are freed.
//
// All valid thread ID numbers are nonzero and positive, and are typically
// small. The thread ID number 0 is used to express a null thread, and the
// thread ID number -1 is used to express the currently running thread.
//

typedef int_fast32_t __PINION_thread_id;

#define __PINION_THREAD_ID_NULL 0
#define __PINION_THREAD_ID_SELF -1

//
// Thread Control and Lifecycle
// 
// Threads have four possible states: free, suspended, ready, and running.
// The thread lifecycle API calls control the transition of threads between
// these states.
// 
// The free state is for threads that have not been allocated yet. The ready
// and running states are for threads that are in the scheduling queue and
// being run, respectively; from an outside thread's view, they are 
// indistinguishable. The suspended state is for threads that cannot be run
// currently, but allows other threads to access register state safely. The
// suspended state may also result from threads causing hardware faults.
//

//
// thread_create - create a new thread
//
// Constructs a new thread and returns its ID number; if something goes wrong,
// it returns THREAD_ID_NULL (i.e. 0). If successful, the thread is initially
// in a suspended state and its register state is undefined.
//
// Returns new thread ID on success, zero on failure.
//

__PINION_thread_id __PINION_thread_create(void);

//
// thread_reap - destroy a thread
//
// Completely frees the (pinion-controlled) state of a suspended thread. The
// thread's ID is then available for reallocation.
//
// Returns true on success, false on failure.
//

bool __PINION_thread_reap(__PINION_thread_id thread);

//
// thread_pause - suspend a running thread
//
// Suspends the given running thread so that its state can be externally
// examined and/or it can be reaped. If thread is THREAD_ID_SELF (i.e. -1)
// then the currently running thread is suspended. The given thread must
// be in a ready or running state to be suspended.
// 
// Parameter thread: thread ID of thread to suspend.
//
// Returns true on success, false on failure.
// 

bool __PINION_thread_pause(__PINION_thread_id thread);

//
// thread_resume - resume a suspended thread
//
// Resumes the given suspended thread. The given thread must be in a suspended
// state.
//
// Parameter thread: thread ID of the thread to resume.
//
// Returns true on success, false on failure.
//

bool __PINION_thread_resume(__PINION_thread_id thread);

//
// thread_yield - yield current thread timeslice
//

void __PINION_thread_yield(void);

//
// thread_exit - complete the current thread and zombify
//

void __PINION_thread_exit(void) __attribute__((noreturn));

//
// Thread State
//
// TODOC
//

typedef uint_fast16_t __PINION_thread_state_class;

#define __PINION_THREAD_STATE_CLASS_EVERYTHING  0xFFFF
#define __PINION_THREAD_STATE_CLASS_REGS        0x00FF
#define __PINION_THREAD_STATE_CLASS_REGS_CALLEE 0x0007
#define __PINION_THREAD_STATE_CLASS_REGS_IP     0x0001
#define __PINION_THREAD_STATE_CLASS_REGS_SP     0x0002
#define __PINION_THREAD_STATE_CLASS_REGS_CALLER 0x0008
#define __PINION_THREAD_STATE_CLASS_REGS_SYSTEM 0x0010

struct __PINION_thread_state {

	// CLASS_REGS_CALLEE (callee-saved)
	uint64_t rip; // CLASS_REGS_IP (instruction pointer)
	uint64_t rsp; // CLASS_REGS_SP (stack pointer)
	uint64_t rbx;
	uint64_t rbp;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;

	// CLASS_REGS_CALLER (caller-saved)
	uint64_t rax;
	uint64_t rcx;
	uint64_t rdx;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;

	// CLASS_REGS_SYSTEM (system-set)
	uint64_t fs;
	uint64_t gs;
	uint64_t cs;
	uint64_t ss;
	uint64_t ds;
	
} __attribute__((packed));

//
// thread_pull_state - read the state of a thread
//

bool 
__PINION_thread_pull_state(
	__PINION_thread_id thread, 
	__PINION_thread_state_class state_class,
	struct __PINION_thread_state *state);

//
// thread_push_state - write the state of a thread
//

bool 
__PINION_thread_push_state(
	__PINION_thread_id thread, 
	__PINION_thread_state_class state_class,
	const struct __PINION_thread_state *state);

//////////////////////////////////////////////////////////////////////////////
// Paging
//

typedef uint_fast8_t __PINION_pagetable_id;
typedef uintptr_t __PINION_virtaddr;
typedef uint64_t __PINION_physaddr;
typedef uint64_t __PINION_pageflags;

#define __PINION_PAGE_VALID         0x0001

#define __PINION_PAGE_READ          0x0010
#define __PINION_PAGE_WRITE         0x0020
#define __PINION_PAGE_EXEC          0x0040
#define __PINION_PAGE_USER          0x0080

#define __PINION_PAGE_ACCESSED      0x0100
#define __PINION_PAGE_DIRTY         0x0200

#define __PINION_PAGE_CACHE_DISABLE 0x1000
#define __PINION_PAGE_WRITE_THROUGH 0x2000

//
// page_set - change page mappings
//

bool __PINION_page_set(__PINION_virtaddr virtual_base, 
	__PINION_physaddr physical_base,
	__PINION_pageflags page_flags, 
	size_t length);

//
// page_get - get translation and page flags of a virtual address
//

struct __PINION_page_content {
	__PINION_physaddr  paddr;
	__PINION_pageflags flags;
} __attribute__((packed));

struct __PINION_page_content __PINION_page_get(
	__PINION_virtaddr virtual_base);

//////////////////////////////////////////////////////////////////////////////
// Page Tables
//
// TODO

//////////////////////////////////////////////////////////////////////////////
// Interrupts
//

//
// Interrupt ID Numbers
//

typedef uint_fast16_t __PINION_interrupt_id;

#define __PINION_INTERRUPT_NULL      0x0000
#define __PINION_INTERRUPT_IRQ(n)    (0x0100 + (n))
#define __PINION_INTERRUPT_PAGEFAULT 0x0080
#define __PINION_INTERRUPT_MISCFAULT 0x0081
#define __PINION_INTERRUPT_ZOMBIE    0x0082

//
// Interrupt Taps
//
// TODOC
//

typedef uint_fast8_t __PINION_tap_index;

#define __PINION_TAP_COUNT 4

//
// interrupt_set_tap
//

bool 
__PINION_interrupt_set_tap(
	__PINION_tap_index index, 
	__PINION_interrupt_id vec);

//
// interrupt_get_tap
//

__PINION_interrupt_id
__PINION_interrupt_get_tap(
	__PINION_tap_index index);

//
// interrupt_reset
//

void 
__PINION_interrupt_reset(
	__PINION_interrupt_id vec);

//
// interrupt_wait
//

__PINION_interrupt_id
	__PINION_interrupt_wait(void);

//////////////////////////////////////////////////////////////////////////////
// Fault Handling
//

//
// fault_get_pagefault - handle a page fault
//
// Returns true on success, false on failure.
//

struct __PINION_pagefault_info {
	__PINION_thread_id thread;
	__PINION_virtaddr  vaddr;
	__PINION_physaddr  paddr;
	__PINION_pageflags flags;
} __attribute__((packed));

bool __PINION_fault_get_pagefault(struct __PINION_pagefault_info *info);

//
// fault_get_zombie - handle a zombie thread
//
// Returns true on success, false on failure.
//

struct __PINION_zombie_info {
	__PINION_thread_id thread;
};

bool __PINION_fault_get_zombie(struct __PINION_zombie_info *info);

//
// fault_get_miscfault - handle an uncategorized hardware fault
//
// Returns true on success, false on failure.
//

struct __PINION_miscfault_info {
	__PINION_thread_id thread;
	uint64_t type;
	uint64_t errcode;
} __attribute__((packed));

bool __PINION_fault_get_miscfault(struct __PINION_miscfault_info *info);

//////////////////////////////////////////////////////////////////////////////
// Physical Memory Map
//
// Pinion builds a map of free physical memory regions (usually passed to it 
// from the bootloader) which it makes available for further memory 
// management. This map is set at boot and will never change for any reason.
// Pinion itself requires memory, so it reserves an arbitrary region of free
// memory for itself; this region is typically very small, but is completely
// implementation-dependent (it does appear as non-free in the memory map)
//
// The memory map is composed of a sequence of free physcial memory regions
// expressed as base and limit physical addresses. These regions are 
// guaranteed to be freely usable from address base to address limit-1, i.e.
// the interval [base, limit). A null region is one with base and limit of 0.
// 
// The indices used to address the non-null regions of the memory map are 
// contiguous and start at index 0. Regions are guaranteed to be disjoint,
// page-aligned, and to have at least one page of non-free memory between 
// them.
//

typedef uint_fast8_t __PINION_memory_map_index;

struct __PINION_memory_map_region {
	__PINION_physaddr base;
	__PINION_physaddr limit;
} __attribute__((packed));

//
// memory_map_read - read a memory map region
//
// Parameter index: index of region to read
//
// Returns the region described by the given index into the pinion memory map,
// or a null region of the index is out of bounds.
//

struct __PINION_memory_map_region __PINION_memory_map_read(
	__PINION_memory_map_index index);

//////////////////////////////////////////////////////////////////////////////
// Loadable Objects
//

//
// In its capacity as a bootloader, pinion allows additional binary data to
// be loaded along with the kernel image in the form of "loadable objects".
// These are akin to GRUB modules.
//
// Unlike GRUB modules, loadable objects are not accessible when the kernel
// starts, and must be loaded explicitly. This has numerous advantages, and is
// possible for pinion because it stays resident unlike a bootloader. Loadable
// objects are intended to essentially form a simple read-only filesystem.
//

typedef uint_fast8_t __PINION_object_id;

struct __PINION_object_stat {
	uint64_t size;
	char name[56];
};

//
// object_stat - read loadable object metadata
//

bool 
__PINION_object_stat(
	__PINION_object_id id, 
	struct __PINION_object_stat *stat);

//
// object_find - locate a loadable object by name
//

__PINION_object_id
__PINION_object_find(
	const char *object_name);

//
// object_read - read the contents of a loadable object into memory
//

uint32_t
__PINION_object_read(
	__PINION_object_id id, 
	uint64_t offset, 
	void *buffer, 
	uint32_t size);

#endif//__PINION_H
