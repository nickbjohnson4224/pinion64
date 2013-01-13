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
global start:function internal
global gdt:data internal
global get_pinion_range:function internal

global read_cr2:function internal

; imported symbols
extern __baseaddr
extern __limitaddr

extern init

extern pmm_init
extern pcx_init
extern idt_init
extern ccb_new
extern tcb_new
extern ccb_load_tcb
extern apic_init
extern load_kernel

extern resume_state_full

LIMIT_CORES equ 1

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

	gdt:
		dq 0x0000000000000000 ; null entry
		dq 0x0020980000000000 ; kernel/pinion code segment
		dq 0x0020920000000000 ; kernel/pinion data segment
		dq 0x0020F80000000000 ; user code segment
		dq 0x0020F20000000000 ; user data segment
		dq 0x0000000000000000 ; reserved for 32-bit code
		dq 0x0000000000000000 ; reserved for 32-bit data
		dq 0x0000000000000000 ; reserved for future use
		dq 0x0000000000000000 ; reserved for future use
		dq 0x0000000000000000 ; reserved for future use
		dq 0x0000000000000000 ; reserved for future use
		dq 0x0000000000000000 ; reserved for future use
		dq 0x0000000000000000 ; reserved for future use
		dq 0x0000000000000000 ; reserved for future use
		dq 0x0000000000000000 ; reserved for future use
		dq 0x0000000000000000 ; reserved for future use

		%rep LIMIT_CORES
		dq 0x0000890000000067 ; TSS for CPU n is descriptor 0x80 + 0x10 * n
		dq 0x0000000000000000
		%endrep

	gdtptr:
		dw (8 * 16 + 16 * LIMIT_CORES) - 1
		dq 0

section .bss

		resb 0x200
	init_stack:

; system initialization
section .text

	; pinion64 entry point
	start:
		
		lea rsp, [rel init_stack - 0x18]
		mov [rsp+0x10], rbx
		mov [rsp+0x08], rcx
		mov [rsp+0x00], rdx

		mov rdi, [rsp+0x00]
		mov rsi, [rsp+0x08]
		mov rdx, [rsp+0x10]

		call gdt_init
		call init

		jmp resume_state_full

	; initialize the global descriptor table
	gdt_init:

		; update GDT pointer and load
		lea rax, [rel gdtptr]
		lea rcx, [rel gdt]
		mov [rax + 2], rcx
		lgdt [rax]

		; reset all segment registers
		mov ax, 0x10
		mov ds, ax
		mov es, ax
		mov gs, ax
		mov fs, ax
		mov ss, ax

		ret

	read_cr2:
		mov rax, cr2
		ret
