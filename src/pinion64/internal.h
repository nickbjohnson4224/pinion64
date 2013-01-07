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
#include <stddef.h>

//
// bootloader stuff
//

// unfold64 memory map structure
struct unfold64_mmap {
	uint64_t total;
	uint64_t count;
	uint64_t entry[];
} __attribute__((packed));

// unfold64 object entry structure
struct unfold64_obje {
	uint64_t base;
	uint32_t size;
	uint32_t type;
	char name[48];
} __attribute__((packed));

// unfold64 object list structure
struct unfold64_objl {
	uint32_t count;
	uint32_t reserved;
	struct unfold64_obje entry[];
} __attribute__((packed));

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
// Pinion API
//

//
// API symbol resolution
//

uint64_t api_resolve(uint8_t apirev, const char *symbol);

//
// configuration (TODO)
//

#define CONFIG_TYPE_PTR 1
#define CONFIG_TYPE_INT 2
#define CONFIG_TYPE_STR 3

void config_set(const char *domain, const char *key, const char *value);
void config_tap(const char *domain, const char *key, int type, void *tap);

//
// locking and synchronization
//

int  mutex_trylock(uint8_t *mutex);
void mutex_acquire(uint8_t *mutex);
void mutex_release(uint8_t *mutex);

//
// memory management
//

void pmm_init(void *mmap);
void *pmm_alloc();
uint64_t pmm_get_pinion_load_addr(void);

void *ccb_alloc();
void ccb_free(void *ccb);

void *tcb_alloc();
void tcb_free(void *tcb);

void *pge_alloc();
void pge_free(void *page);

//
// paging structures
//

#define UPF_PRESENT  0x0001
#define UPF_WRITE    0x0002
#define UPF_EXEC     0x0004
#define UPF_USER     0x0008

#define UPF_ACCESSED 0x0010
#define UPF_DIRTY    0x0020

#define UPF_CACHE_DISABLE 0x1000
#define UPF_WRITE_THROUGH 0x2000

#define PF_PRES  0x1   // is present
#define PF_WRITE 0x2   // is writable
#define PF_USER  0x4   // is user accessible
#define PF_PWT   0x8   // write-through
#define PF_PCD   0x10  // cache disable
#define PF_ACCS  0x20  // accessed
#define PF_DIRTY 0x40  // dirty
#define PF_LARGE 0x80  // large page
#define PF_GLOBL 0x100 // global
#define PF_NX    (1ULL << 63) // no-execute
#define PF_BITS 0x8000000000000FFFULL // bits used as flags

void      pcx_init(void);

uint64_t *pcx_new(uint32_t *pcx_id);
void      pcx_del(uint64_t *pcx);

uint64_t *pcx_get(uint32_t pcx_id);

uint64_t  pcx_get_trans(uint64_t *pcx, uint64_t addr);
uint64_t  pcx_get_frame(uint64_t *pcx, uint64_t base);
uint64_t  pcx_get_flags(uint64_t *pcx, uint64_t base);

int       pcx_set_frame(uint64_t *pcx, uint64_t base, uint64_t size, uint64_t frame, uint64_t flags);
int       pcx_set_flags(uint64_t *pcx, uint64_t base, uint64_t size, uint64_t flags);

void      pcx_switch(uint64_t *pcx);

void *pmm_vaddr(uint64_t paddr);

//
// event sets
//

//
// processors / CCB
//

// Task State Segment (exactly 104 bytes)
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
} __attribute__((packed));

// CPU control block (exactly 4096 bytes)
struct ccb {

	// core ID (offset 0x00)
	uint32_t id;
	uint32_t reserved0;

	// call stack pointer (offset 0x08)
	void *cstack;

	// interrupt stack pointer (offset 0x10)
	void *istack;

	// active thread control block (offset 0x18)
	struct tcb *active_tcb;

	// paging contexts (offset 0x20)
	void *active_pcx;
	uint32_t active_pcx_id;
	uint32_t active_root_pcx_id;

	uint8_t padding0[952];

	// APIC ID of core (offset 0x3E0)
	uint32_t apic_id;
	uint32_t reserved1;

	// IOAPIC configuration space (offset 0x3F0)
	uint32_t *ioapic_ptr;

	// LAPIC configuration space (offset 0x3F8)
	uint32_t *lapic_ptr;

	// system call stack (offset 0x400)
	uint8_t cstack_space[1024];

	// interrupt stack (offset 0x800)
	uint8_t istack_space[1024];

	// task state segment (offset 0xC00)
	struct tss tss;

	uint8_t padding1[920];

} __attribute__((packed));

struct ccb *ccb_new(void);
struct ccb *ccb_get(uint32_t core_id);
struct ccb *ccb_get_self(void);

int ccb_load_tcb(struct tcb *tcb);
struct tcb *ccb_unload_tcb(void);

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

	// thread state (8 bytes)
	uint16_t state_reserved;
	uint16_t state;

	// remainder valid if TSFLAG_VALID
	uint32_t id;

	// running thread information (4 bytes)
	// valid if TSFLAG_RUNNING
	uint32_t running_cpu;

	// waiting thread information (4 bytes)
	// valid if TSFLAG_WAITING
	uint16_t wait_type;
	uint16_t wait_evset_id;

	// reserved for future use
	uint64_t reserved[4];

	// remainder valid for external reads if TSFLAG_SUSPENDED

	// paging context (8 bytes)
	uint32_t pcx_id;
	uint32_t pcx_reserved;

	// fault address (most important for page faults)
	uint64_t fault_addr;

	// saved thread continuation (192 bytes)
	uint64_t gs;
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

	void    *xstate; // state for (F)XSAVE/(F)XRSTOR

	uint32_t ivect; // interrupt vector number
	uint32_t saved; // set of saved register types
	uint64_t error; // error code (if applicable)
	uint64_t rip;
	uint64_t cs;
	uint64_t rflags;
	uint64_t rsp;
	uint64_t ss;
	
} __attribute__((packed));

struct tcb *tcb_new(void);
void        tcb_del(struct tcb *);
struct tcb *tcb_get(uint32_t thread_id);

#endif//__PINION64_INTERNAL_H
