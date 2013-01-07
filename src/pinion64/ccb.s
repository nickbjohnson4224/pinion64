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
global ccb_new:function internal
global ccb_get:function internal
global ccb_get_self:function internal

global ccb_load_tcb:function internal
global ccb_unload_tcb:function internal

; imported symbols
extern gdt
extern ccb_alloc
extern mutex_trylock
extern mutex_acquire
extern mutex_release
extern pcx_get
extern __baseaddr

extern debug_ptr

LIMIT_CORES equ 1

section .bss

	ccb_table:
		resq LIMIT_CORES

	ccb_table_lock:
		resb 1

section .text

	ccb_new:

		sub rsp, 8
		mov [rsp], rbx

		; acquire table lock
		lea rdi, [rel ccb_table_lock]
		call mutex_acquire

		; allocate core ID in table
		lea rax, [rel ccb_table]
		xor rcx, rcx

		.loopa:
		mov rdx, [rax+rcx*8]
		test rdx, rdx
		jz .donea
		cmp rcx, LIMIT_CORES
		jae .faila
		jmp .loopa

		.faila:
		lea rdi, [rel ccb_table_lock]
		call mutex_release
		mov rbx, [rsp]
		add rsp, 8
		xor rax, rax
		ret

		.donea:
		mov rbx, rcx

		; allocate the CCB
		call ccb_alloc
		lea rcx, [rel ccb_table]
		mov [rcx + rbx * 8], rax

		; release table lock
		lea rdi, [rel ccb_table_lock]
		call mutex_release

		; set core ID
		lea rax, [rel ccb_table]
		mov rcx, [rax + rbx * 8]
		mov [rcx], ebx

		; set up stack pointers
		lea rax, [rcx+0x800]
		mov [rcx+0x08], rax
		lea rax, [rcx+0xC00]
		mov [rcx+0x10], rax

		; set active TCB and PCX to null
		xor rax, rax
		mov [rcx+0x18], rax
		mov [rcx+0x20], rax
		mov [rcx+0x28], rax

		; set up TSS
		lea rax, [rcx+0xC00]
		lea rdx, [rel gdt + 0x80]
		mov rsi, rbx
		shl rsi, 4
		add rdx, rsi
		
		mov [rdx+2], ax
		shr rax, 16
		mov [rdx+4], al
		mov [rdx+7], ah
		shr rax, 16
		mov [rdx+8], eax

		; load TSS
		mov ax, bx
		shl ax, 4
		add ax, 0x80
		ltr ax

		; set GS to point to CCB
		mov rax, rcx
		mov rdx, rcx
		shr rdx, 32
		mov ecx, 0xC0000102
		wrmsr
		mov ecx, 0xC0000101
		wrmsr

		mov rbx, [rsp]
		add rsp, 8
		ret

	ccb_get:

		cmp rdi, LIMIT_CORES
		jae .fail

		lea rcx, [rel ccb_table]
		mov rax, [rcx + rdi * 8]
		ret

		.fail:
		xor rax, rax
		ret

	ccb_get_self:

		xor rax, rax
		mov eax, [gs:0x00]
		lea rcx, [rel ccb_table]
		mov rax, [rcx + rax * 8]
		ret

	ccb_unload_tcb:

		; check that the core is busy
		mov rsi, [gs:0x18]
		test rsi, rsi
		jz .fail

		; set TCB state to READY
		mov ax, 1
		mov [rsi+2], ax
		mov eax, -1
		mov [rsi+0x10], eax

		; remove TCB from CCB
		xor rax, rax
		mov [gs:0x18], rax

		; unlock TCB mutex
		mov [rsi], al

		; return TCB
		mov rax, rsi
		ret

		.fail:
		xor rax, rax
		ret

	ccb_load_tcb:

		; check that the core is not busy
		mov rsi, [gs:0x18]
		test rsi, rsi
		jnz .fail

		; check that TCB state is READY
		mov ax, [rdi+2]
		cmp ax, 1
		jne .fail

		; set TCB state to RUNNING
		mov ax, 3
		mov [rdi+2], ax
		mov eax, [gs:0x00]
		mov [rdi+0x10], eax

		; insert TCB into CCB, TSS.RSP0, TSS.IST1
		mov [gs:0x18], rdi
		lea rax, [rdi+0x100]
		mov [gs:0xC04], rax
		mov [gs:0xC24], rax

		; check if PCX switch is needed
		mov rcx, [gs:0x20]
		test rcx, rcx
		jz .pcxswitch ; CCB active PCX is null
		mov eax, [rdi+0x30]
		test eax, eax
		jz .nopcxswitch ; TCB PCX is root
		mov ecx, [gs:0x28]
		cmp eax, ecx
		je .nopcxswitch ; CCB and TCB PCX's match

		.pcxswitch:
		xor rax, rax
		mov eax, [rdi+0x30]
		mov [gs:0x28], eax

		mov rdi, rax
		call pcx_get
		lea rcx, [rel __baseaddr - 0x200000]
		sub rax, rcx

		mov cr3, rax
		mov [gs:0x20], rax

		.nopcxswitch:
		xor rax, rax
		ret

		.fail:
		mov rax, 1
		ret
