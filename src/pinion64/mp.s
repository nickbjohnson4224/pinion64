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
global find_mptable:function internal

; imported symbols
extern pmm_vaddr

section .text

find_mptable:
	
	xor rdi, rdi
	call pmm_vaddr

	mov rdx, rax
	mov eax, 0x5F504D5F

	lea rcx, [rdx + 0xF00000]
	
	.loop:
	add rdx, 16
	cmp rdx, rcx
	jae .fail
	mov eax, [rdx]
	cmp eax, 0x5F504D5F
	jne .loop

	mov al, [rdx]
	add al, [rdx+1]
	add al, [rdx+2]
	add al, [rdx+3]
	add al, [rdx+4]
	add al, [rdx+5]
	add al, [rdx+6]
	add al, [rdx+7]
	add al, [rdx+8]
	add al, [rdx+9]
	add al, [rdx+10]
	add al, [rdx+11]
	add al, [rdx+12]
	add al, [rdx+13]
	add al, [rdx+14]
	add al, [rdx+15]

	test al, al
	jnz .loop

	mov rax, rdx
	ret

	.fail:
	xor rax, rax
	ret
