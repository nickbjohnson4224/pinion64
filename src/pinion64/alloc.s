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

; TODO - upgrade to proper multiprocessor version
; TODO - do proper sub-allocations

; imported symbols
extern pmm_alloc
extern pmm_base
extern mutex_acquire
extern mutex_release

%macro allocator 2

	; exported symbols
	global %1_alloc
	global %1_free

	section .data

		%1_list:
			dq 0

		%1_lock:
			dq 0

	section .text

		%1_alloc:

			push rbx
			
			lea rdi, [rel %1_lock]
			call mutex_acquire

			.retry:
			mov rax, [rel %1_list]
			test rax, rax
			jz .pull

			mov rcx, [rax]
			mov [rel %1_list], rcx
			mov rbx, rax

			lea rdi, [rel %1_lock]
			call mutex_release

			mov rax, rbx
			pop rbx
			ret

			.pull:
			call pmm_alloc

			test rax, rax
			jz .fail

			mov rcx, [rel %1_list]
			mov [rax], rcx
			mov [rel %1_list], rax

			jmp .retry

			.fail:
			lea rdi, [rel %1_lock]
			call mutex_release
			xor rax, rax
			pop rbx
			ret

		%1_free:

			push rdi

			lea rdi, [rel %1_lock]
			call mutex_acquire

			pop rdi

			mov rax, [rel %1_list]
			mov [rdi], rax
			mov [rel %1_list], rdi

			lea rdi, [rel %1_lock]
			call mutex_release

			ret
%endmacro

allocator tcb, 256
allocator ccb, 1024
allocator pge, 4096
allocator pcx, 4096
