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

[bits 32]

section .pagedata

pt4:
	dd (pt30 + 3)
	dd 0
	times 510 dq 0
	dd (pt31 + 3)
	dd 0

pt30:
	dd (pt2 + 3)
	dd 0
	times 511 dq 0

pt31:
	times 511 dq 0
	dd (pt2 + 3)
	dd 0

pt2:
%assign i 0
%rep 512
	dd (i << 21) | 0x83
	dd 0
%assign i i+1
%endrep

section .data

gdt64:
	dq 0x0000000000000000
	dq 0x0020980000000000
	dq 0x0000900000000000

gdt64ptr:
	dw 0x17
	dd gdt64
	dd 0

section .text

global init_enter_long
init_enter_long:

	; enable PAE paging
	mov eax, cr4
	or  eax, 1 << 5
	mov cr4, eax

	; set long mode bit
	mov ecx, 0xC0000080
	rdmsr
	or  eax, 1 << 8
	wrmsr

	; enable paging
	mov eax, pt4
	mov cr3, eax
	mov eax, cr0
	or  eax, 1 << 31
	mov cr0, eax

	; load 64-bit GDT
	lgdt [gdt64ptr]
	jmp 0x08:enter64

enter64:
	; mov rax, [esp+4]
	db 0x67, 0x48, 0x8b, 0x44, 0x24, 0x04
	; jmp rax
	db 0xff, 0xe0
