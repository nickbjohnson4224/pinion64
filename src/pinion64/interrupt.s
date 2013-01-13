; Copyright (C) 2012 Nick Johnson <nickbjohnson4224 at gmail.com>
; 
; Permission to use, copy, modify, and distribute this software for any
; purpose with or without fee is hereby granted, provided that the above
; copyright notice and this permission notice appear in all copies.
; 
; THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
; WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
; MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
; ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
; WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
; ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
; OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

[bits 64]

; exported symbols
global idt_init:function internal
global apic_init:function internal

global resume_state_full:function internal

; imported symbols
extern fault
extern pge_alloc
extern __baseaddr
extern ioapic_init

extern debug_ptr

section .data

	idt:
		dq 0

	idtptr:
		dw (16 * 256) - 1
		dq 0

section .text

	; macros for generating fault handler stubs
	%macro fault_stub_err 1
	fault_%1:
		push %1
		jmp save_state_full
	%endmacro

	%macro fault_stub_noerr 1
	fault_%1:
		push 0
		push %1
		jmp save_state_full
	%endmacro

	; macro for generating IRQ handler stubs
	%macro irq_stub 1
	irq_%1:
		push 0
		push %1
		jmp save_state_full
	%endmacro

	; fault handler stubs
	fault_stub_noerr 0  ; #DE - divide error
	fault_stub_noerr 1  ; #DB - debug
	fault_stub_noerr 2  ; NMI - non-maskable interrupt
	fault_stub_noerr 3  ; #BP - breakpoint
	fault_stub_noerr 4  ; #OF - overflow
	fault_stub_noerr 5  ; #BR - bound range exceeded
	fault_stub_noerr 6  ; #UD - invalid opcode
	fault_stub_noerr 7  ; #NM - device not available
	fault_stub_noerr 8  ; #DF - double fault
	fault_stub_err   10 ; #TS - invalid TSS
	fault_stub_err   11 ; #NP - segment not present
	fault_stub_err   12 ; #SS - stack fault
	fault_stub_err   13 ; #GP - general protection fault
	fault_stub_err   14 ; #PF - page fault
	fault_stub_noerr 16 ; #MF - x87 floating point error
	fault_stub_err   17 ; #AC - alignment check
	fault_stub_noerr 18 ; #MC - machine check
	fault_stub_noerr 19 ; #XM - SIMD floating point error

	%assign i 32
	%rep 256-32
		irq_stub i
	%assign i i+1
	%endrep

	; generic interrupt state saving
	save_state_full:	

		sub rsp, 0x88
		mov [rsp+0x08], rax
		mov [rsp+0x10], rcx
		mov [rsp+0x18], rbx
		mov [rsp+0x20], rdx
		mov [rsp+0x28], rdi
		mov [rsp+0x30], rsi
		mov [rsp+0x38], rbp
		mov [rsp+0x40], r8
		mov [rsp+0x48], r9
		mov [rsp+0x50], r10
		mov [rsp+0x58], r11
		mov [rsp+0x60], r12
		mov [rsp+0x68], r13
		mov [rsp+0x70], r14
		mov [rsp+0x78], r15

		swapgs
		lea rdi, [rsp-0x40]
		mov rsi, [gs:0x30]
		mov rsp, [gs:0x10]
		call fault
		
	resume_state_full:

		mov rsp, [gs:0x18]
		add rsp, 0x40
		swapgs

		mov rax, [rsp+0x08]
		mov rcx, [rsp+0x10]
		mov rbx, [rsp+0x18]
		mov rdx, [rsp+0x20]
		mov rdi, [rsp+0x28]
		mov rsi, [rsp+0x30]
		mov rbp, [rsp+0x38]
		mov r8,  [rsp+0x40]
		mov r9,  [rsp+0x48]
		mov r10, [rsp+0x50]
		mov r11, [rsp+0x58]
		mov r12, [rsp+0x60]
		mov r13, [rsp+0x68]
		mov r14, [rsp+0x70]
		mov r15, [rsp+0x78]

		add rsp, 0x98
		iretq

	; add a trap vector to the IDT
	idt_set_trap_vector:
		mov rcx, [rel idt]
		mov rax, rsi
		shl rax, 4
		add rcx, rax

		mov rax, rdi
		shr rax, 16
		mov [rcx+0x00], di
		mov [rcx+0x02], word 0x0008
		mov [rcx+0x04], word 0x8F01
		mov [rcx+0x06], ax
		shr rax, 16
		mov [rcx+0x08], eax

		ret

	; add an IRQ vector to the IDT
	idt_set_irq_vector:
		mov rcx, [rel idt]
		mov rax, rsi
		shl rax, 4
		add rcx, rax

		mov rax, rdi
		shr rax, 16
		mov [rcx+0x00], di
		mov [rcx+0x02], word 0x0008
		mov [rcx+0x04], word 0x8E01
		mov [rcx+0x06], ax
		shr rax, 16
		mov [rcx+0x08], eax

		ret

	; initialize the interrupt descriptor table
	idt_init:

		; allocate IDT
		call pge_alloc
		mov [rel idt], rax

		%macro build_fault 1
			lea rdi, [rel fault_%1]
			mov rsi, %1
			call idt_set_trap_vector
		%endmacro

		%macro build_irq 1
			lea rdi, [rel irq_%1]
			mov rsi, %1
			call idt_set_irq_vector
		%endmacro

		; build fault handlers
		build_fault 0
		build_fault 1
		build_fault 2
		build_fault 3
		build_fault 4
		build_fault 5
		build_fault 6
		build_fault 7
		build_fault 8
		build_fault 10
		build_fault 11
		build_fault 12
		build_fault 13
		build_fault 14
		build_fault 16
		build_fault 17
		build_fault 18
		build_fault 19

		; build IRQ handlers
		%assign i 32
		%rep 256-32
			build_irq i
		%assign i i+1
		%endrep

		; update IDT pointer and load
		lea rax, [rel idtptr]
		mov rcx, [rel idt]
		mov [rax+2], rcx
		lidt [rax]

		ret

	apic_init:

		; activate LAPIC
		mov ecx, 0x1B
		rdmsr
		or eax, 0x800
		wrmsr

		; set LAPIC pointer
		lea rax, [rel __baseaddr - 0x200000 + 0x3EE00000]
		mov [gs:0x3F8], rax

		; get APIC ID
		mov eax, [rax+0x20]
		mov [gs:0x3E8], eax

		; configure IOAPIC
		mov rdi, [gs:0x3F8]
		call ioapic_init

		; set IOAPIC pointer
		mov [gs:0x3F0], rax

		ret
