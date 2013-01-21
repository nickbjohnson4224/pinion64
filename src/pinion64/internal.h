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
// init stuff
//

void idt_init(void);
void load_kernel(struct unfold64_objl *objl);

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

uint64_t api_resolve(const char *symbol);

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

#define PF_PRES  0x1   // is present
#define PF_WRITE 0x2   // is writable
#define PF_USER  0x4   // is user accessible
#define PF_PWT   0x8   // write-through
#define PF_PCD   0x10  // cache disable
#define PF_ACCS  0x20  // accessed
#define PF_DIRTY 0x40  // dirty
#define PF_LARGE 0x80  // large page
#define PF_GLOBL 0x100 // global
#define PF_VALID 0x200 // valid (pinion-defined)
#define PF_NREAD 0x400 // no-read (pinion-defined)
#define PF_PTCOW 0x800 // pagetable copy-on-write (pinion-defined)
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

struct lapic_reg {
	uint32_t value;
	uint32_t ___[3];
} __attribute__((packed));

struct lapic_regs {
	
	uint32_t reserved00[8];

	// Local APIC ID Register (offset 0x10, RO)
	uint32_t id;
	uint32_t reserved01[3];

	// Local APIC Version Register (offset 0x20, RO)
	uint32_t version;
	uint32_t reserved02[3];

	uint32_t reserved03[16];

	// Task Priority Register (TPR) (offset 0x80, RW)
	uint32_t task_priority;
	uint32_t reserved04[3];

	// Arbitration Priority Register (APR) (offset 0x90, RO)
	uint32_t arbitration_priority;
	uint32_t reserved05[3];

	// Processor Priority Register (PPR) (offset 0xA0, RO)
	uint32_t processor_priority;
	uint32_t reserved06[3];

	// EOI Register (offset 0xB0, WO)
	uint32_t eoi;
	uint32_t reserved07[3];

	// Remote Read Register (RRD) (offset 0xC0, RO)
	uint32_t remote_read;
	uint32_t reserved08[3];

	// Logical Destination Register (offset 0xD0, RW)
	uint32_t logical_destination;
	uint32_t reserved09[3];

	// Destination Format Register (offset 0xE0, RW)
	uint32_t destination_format;
	uint32_t reserved10[3];

	// Spurious Interrupt Vector Register (offset 0xF0, RW)
	uint32_t spurious_interrupt_vector;
	uint32_t reserved11[3];

	// In-Service Register (ISR) (offset 0x100, RO)
	struct lapic_reg in_service[8];

	// Trigger-Mode Register (TMR) (offset 0x180, RO)
	struct lapic_reg trigger_mode[8];

	// Interrupt Request Register (IRR) (offset 0x200, RO)
	struct lapic_reg interrupt_request[8];

	// Error Status Register (offset 0x280, RO)
	uint32_t error_status;
	uint32_t reserved12[3];

	uint32_t reserved13[24];

	// LVT CMCI Register (offset 0x2F0, RW)
	uint32_t lvt_cmci;
	uint32_t reserved14[3];

	// Interrupt Command Register (ICR) bits 0-31 (offset 0x300, RW)
	uint32_t interrupt_command_0;
	uint32_t reserved15[3];

	// Interrupt Command Register (ICR) bits 32-63 (offset 0x310, RW)
	uint32_t interrupt_command_1;
	uint32_t reserved16[3];

	// LVT Timer Register (offset 0x320, RW)
	uint32_t lvt_timer;
	uint32_t reserved17[3];

	// LVT Thermal Sensor Register (offset 0x330, RW)
	uint32_t lvt_thermal_sensor;
	uint32_t reserved18[3];

	// LVT Performance Monitoring Counters Register (offset 0x340, RW)
	uint32_t lvt_performance_monitoring_counters;
	uint32_t reserved19[3];

	// LVT LINT0 Register (offset 0x350, RW)
	uint32_t lvt_lint0;
	uint32_t reserved20[3];

	// LVT LINT1 Register (offset 0x360, RW)
	uint32_t lvt_lint1;
	uint32_t reserved21[3];

	// LVT Error Register (offset 0x370, RW)
	uint32_t lvt_error;
	uint32_t reserved22[3];

	// Initial Count Register (for Timer) (offset 0x380, RW)
	uint32_t timer_initial_count;
	uint32_t reserved23[3];

	// Current Count Register (for Timer) (offset 0x390, RO)
	uint32_t timer_current_count;
	uint32_t reserved24[3];

	uint32_t reserved25[16];

	// Divide Configuration Register (for Timer) (offset 0x3E0, RW)
	uint32_t timer_divide_configuration;
	uint32_t reserved26[3];

	uint32_t reserved27[4];

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

	// this ccb (offset 0x30)
	struct ccb *self;

	uint8_t padding0[944];

	// APIC ID of core (offset 0x3E0)
	uint32_t apic_id;
	uint32_t reserved1;

	// IOAPIC configuration space (offset 0x3F0)
	uint32_t *ioapic_ptr;

	// LAPIC configuration space (offset 0x3F8)
	volatile struct lapic_regs *lapic;

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

#define TCB_STATE_FREE              0 // free
#define TCB_STATE_ZOMBIE            1 // zombie
#define TCB_STATE_SUSPENDEDWAITING  2 // suspended waiting
#define TCB_STATE_SUSPENDED         3 // suspended
#define TCB_STATE_WAITING           4 // waiting
#define TCB_STATE_QUEUED            5 // queued
#define TCB_STATE_RUNNING           6 // running
#define TCB_STATE_NULL              7 // nonexistent (not used internally)

// Extended State
struct xstate {
	uint8_t data[512];
};

// Thread Control Block (exactly 256 bytes)
struct tcb {

	uint16_t reserved0;

	// thread state
	uint16_t state; // (offset 0x02)

	// remainder valid if not TCB_STATE_FREE
	
	// thread ID
	uint32_t id; // (offset 0x04)

	// running thread information

	// valid if TCB_STATE_RUNNING
	uint32_t running_cpu; // (offset 0x08)

	// waiting thread information
	// valid if TCB_STATE_WAITING or TCB_STATE_SUSPENDEDWAITING
//	uint16_t wait_type;     // (offset 0x0C)
//	uint16_t wait_evset_id; // (offset 0x0E)

	uint16_t wait_event[6];

	// reserved for future use
	uint64_t reserved1[2];

	// next TCB in scheduler or waitlist
	struct tcb *next;

	// remainder valid for external reads if TCB_STATE_SUSPENDED 
	// or TCB_STATE_SUSPENDEDWATITING

	// paging context
	uint32_t pcx_id; // (offset 0x30)
	uint32_t pcx_reserved; // (offset 0x34)

	// fault address (offset 0x38)
	uint64_t fault_addr;

	// saved thread continuation (192 bytes) (offset 0x40)
	uint64_t gs;  // (offset 0x40)
	uint64_t rax; // (offset 0x48)
	uint64_t rcx; // (offset 0x50)
	uint64_t rbx; // (offset 0x58)
	uint64_t rdx; // (offset 0x60)
	uint64_t rdi; // (offset 0x68)
	uint64_t rsi; // (offset 0x70)
	uint64_t rbp; // (offset 0x78)
	uint64_t r8;  // (offset 0x80)
	uint64_t r9;  // (offset 0x88)
	uint64_t r10; // (offset 0x90)
	uint64_t r11; // (offset 0x98)
	uint64_t r12; // (offset 0xA0)
	uint64_t r13; // (offset 0xA8)
	uint64_t r14; // (offset 0xB0)
	uint64_t r15; // (offset 0xB8)

	void    *xstate; // state for (F)XSAVE/(F)XRSTOR (offset 0xC0)

	uint32_t ivect;  // interrupt vector number (offset 0xC8)
	uint32_t saved;  // set of saved register types (offset 0xC4)
	uint64_t error;  // error code (if applicable) (offset 0xD0)
	uint64_t rip;    // (offset 0xD8)
	uint64_t cs;     // (offset 0xE0)
	uint64_t rflags; // (offset 0xE8)
	uint64_t rsp;    // (offset 0xF0)
	uint64_t ss;     // (offset 0xF8)
	
} __attribute__((packed));

struct tcb *tcb_new(void);
void        tcb_del(struct tcb *);
struct tcb *tcb_get(uint32_t thread_id);

void scheduler_add_tcb(struct tcb *tcb);
int  scheduler_rem_tcb(struct tcb *tcb);
void scheduler_schedule();

#endif//__PINION64_INTERNAL_H
