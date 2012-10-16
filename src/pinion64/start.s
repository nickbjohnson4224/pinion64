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
global start
global get_pinion_range

; imported symbols
extern __baseaddr
extern __limitaddr
extern init
extern fault

section .text

	get_pinion_range:
		lea rax, [rel __baseaddr]
		and rax, ~0xFFF
		mov [rdi], rax
		lea rax, [rel __limitaddr]
		neg rax
		and rax, ~0xFFF
		neg rax
		mov [rsi], rax
		ret

section .data

	idtptr:
		dw (16 * 256) - 1
		dq 0

	gdt:
		dq 0x0000000000000000 ; null entry
		dq 0x0020980000000000 ; kernel code segment
		dq 0x0020920000000000 ; kernel data segment
		dq 0x0020F80000000000 ; user code segment
		dq 0x0020F20000000000 ; user data segment
		dq 0x0000000000000000 ; reserved for 32-bit code
		dq 0x0000000000000000 ; reserved for 32-bit data
		dq 0x0000890000000080 ; TSS for CPU 0
		dq 0x0000000000000000

	gdtptr:
		dw (8 * 9) - 1
		dq 0

section .bss

	idt:
		resb 0x1000

	ccb0:

		cpu0_cstack:
			resb 0x400

		cpu0_istack:
			resb 0x400

		cpu0_tss:
			resb 0x80

		resb 0x380

		resb 0x200
	init_stack:

; system initialization
section .text

	; pinion64 entry point
	start:
		
		lea rsp, [rel init_stack]
		push rbx
		push rcx
		push rdx

		call init_gdt
		call init_idt
		call init_tss

		pop rcx
		pop rdx
		pop rsi
		lea rdi, [rel __baseaddr]
		call init

		sti
		hlt
		jmp $

	; initialize the global descriptor table
	init_gdt:

		; update GDT pointer and load
		lea rax, [rel gdtptr]
		lea rcx, [rel gdt]
		mov [rax + 2], rcx
		lgdt [rax]

		; reset all segment registers
		mov ax, 0x10
		mov ds, ax
		mov es, ax
		mov fs, ax
		mov gs, ax
		mov ss, ax

		ret

	; initialize the task state segment
	init_tss:

		lea rax, [rel cpu0_tss]
		lea rcx, [rel gdt + 0x38]
		mov [rcx], ax
		shr rax, 16
		mov [rcx+4], al
		mov [rcx+7], ah
		shr rax, 16
		mov [rcx+8], eax

		mov ax, 0x38
		ltr ax

		ret

; interrupt handling
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
	fault_stub_err   8  ; #DF - double fault
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

		lea rdi, [rsp-0x40]

		lea rsp, [rel cpu0_istack + 0x400]
;		mov rsp, [rsp]

		call fault

		mov rbp, rsp
		lea rsp, [rax+0x40]
		mov [rsp], rbp       ; set cpu stack field in thread

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

		add rsp, 0x90
		iretq

	; add a trap vector to the IDT
	idt_set_trap_vector:
		lea rcx, [rel idt]
		mov rax, rsi
		shl rax, 4
		add rcx, rax

		mov rax, rdi
		shr rax, 16
		mov [rcx+0x00], di
		mov [rcx+0x02], word 0x0008
		mov [rcx+0x04], word 0x8F00
		mov [rcx+0x06], ax
		shr rax, 16
		mov [rcx+0x08], eax

		ret

	; add an IRQ vector to the IDT
	idt_set_irq_vector:
		lea rcx, [rel idt]
		mov rax, rsi
		shl rax, 4
		add rcx, rax

		mov rax, rdi
		shr rax, 16
		mov [rcx+0x00], di
		mov [rcx+0x02], word 0x0008
		mov [rcx+0x04], word 0x8E00
		mov [rcx+0x06], ax
		shr rax, 16
		mov [rcx+0x08], eax

		ret

	; initialize the interrupt descriptor table
	init_idt:

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

		%assign i 32
		%rep 256-32
			build_irq i
		%assign i i+1
		%endrep

		; update IDT pointer and load
		lea rax, [rel idtptr]
		lea rcx, [rel idt]
		mov [rax+2], rcx
		lidt [rax]

		ret
