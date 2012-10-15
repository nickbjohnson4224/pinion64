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

#ifndef __PINION64_INTERNAL_H
#define __PINION64_INTERNAL_H

#include <stdint.h>

//
// logging
//

// logging levels
#define INFO  0
#define INIT  1
#define DEBUG 2
#define ERROR 3

// logging function of brilliant design
void log(int level, const char *fmt, ...);

//
// locking and synchronization
//

//
// physical memory management
//

#define PMMAP_NOTRAM 0
#define PMMAP_UNUSED 1
#define PMMAP_PINION 2
#define PMMAP_SYSTEM 3

struct pmmap_entry {
	uint32_t type;
	uint64_t base;
	uint64_t size;
};

struct pmmap {
	uint64_t count;
	struct pmmap_entry *entry;
};

struct pmmap *pmmap_get  (void);
void          pmmap_build(void *unfold_mmap)

//
// paging structures
//

//
// event sets
//

//
// processors / CCB
//

// Task State Segment (exactly 128 bytes)
struct tss {
	uint32_t reserved0;
	uint64_t rsp0;
	uint64_t rsp1;
	uint64_t rsp2;
	uint32_t reserved1;
	uint32_t reserved2;
	uint64_t ist1;
	uint64_t ist2;
	uint64_t ist3;
	uint64_t ist4;
	uint64_t ist5;
	uint64_t ist6;
	uint64_t ist7;
	uint32_t reserved3;
	uint32_t reserved4;
	uint16_t reserved5;
	uint16_t iopb_base;

	uint8_t padding[24];
} __attribute__((packed));

// CPU control block (exactly 4096 bytes)
struct ccb {

	// system call stack
	uint8_t cstack[1024];

	// interrupt stack
	uint8_t istack[1024];

	// task state segment
	struct tss tss;

	// active thread control block
	struct tcb *active_tcb;

	// active paging context
	void *active_pcx;

} __attribute__((packed));

//
// threading / TCB
//

#define TSTATE_FR 0x0 // free (unallocated)
#define TSTATE_RE 0x1 // ready
#define TSTATE_RU 0x3 // running
#define TSTATE_WA 0x5 // waiting
#define TSTATE_SR 0x9 // suspended ready
#define TSTATE_SW 0xD // suspended waiting

#define TSFLAG_VALID     0x01
#define TSFLAG_RUNNING   0x02
#define TSFLAG_WAITING   0x04
#define TSFLAG_SUSPENDED 0x08

// Extended State
struct xstate {
	uint8_t data[512];
};

// Thread Control Block (exactly 256 bytes)
struct tcb {

	// thread state and locking
	uint16_t state;
	uint16_t flags;
	uint32_t mutex;

	// remainder valid if TSFLAG_VALID

	// running thread information (8 bytes)
	// valid if TSFLAG_RUNNING
	uint16_t running_cpu;
	uint16_t running_reserved;

	// waiting thread information (8 bytes)
	// valid if TSFLAG_WAITING
	uint16_t wait_type;
	uint16_t wait_evset_id;

	// reserved for future use
	uint64_t reserved[4];

	// remainder valid for external reads if TSFLAG_SUSPENDED

	// paging context (8 bytes)
	uint16_t pctx_id;
	uint16_t pctx_reserved[3];

	// fault address (most important for page faults)
	uint64_t fault_addr;

	// saved thread continuation (192 bytes)
	void *cpu_stack; // pointer to CPU interrupt stack
	uint64_t rax;
	uint64_t rcx;
	uint64_t rbx;
	uint64_t rdx;
	uint64_t rdi;
	uint64_t rsi;
	uint64_t rbp;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
	void    *xstate; // state for XSAVE/XRSTOR

	uint32_t ivect; // interrupt vector number
	uint32_t saved; // set of saved register types
	uint64_t error; // error code (if applicable)
	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss;
	
} __attribute__((packed));

#endif//__PINION64_INTERNAL_H
