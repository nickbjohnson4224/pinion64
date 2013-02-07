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

%macro apifunc 1
	global __PINION_%1:function default
	global apientry_%1:function default
	extern apilogic_%1

	section .text

	__PINION_%1:
	apientry_%1:
		cli
		swapgs
		mov r11, rsp
		mov rsp, [gs:0x10]
		push r11
		call apilogic_%1
		pop rsp
		swapgs
		sti
		ret

%endmacro

%macro apifunc_then_yield 2
	global __PINION_%1:function default
	global apientry_%1:function default
	extern apilogic_%1

	section .text

	__PINION_%1:
	apientry_%1:
		cli
		swapgs
		mov r11, rsp
		mov rsp, [gs:0x10]
		push r11
		call apilogic_%1
		pop rsp
		swapgs
		sti

		; 0 - yield if suspended or zombie
		; 1 - yield if preempted
		; 2 - yield unconditionally

		mov rdi, %2 
		int 0xFF
		ret

%endmacro

; thread API -----------------------------------------------------------------

apifunc_then_yield thread_yield, 2
apifunc_then_yield thread_create, 1
apifunc_then_yield thread_pause, 0
apifunc_then_yield thread_resume, 1
apifunc_then_yield thread_exit, 0

; interrupt API --------------------------------------------------------------

apifunc interrupt_set_tap
apifunc interrupt_get_tap
apifunc interrupt_reset
apifunc_then_yield interrupt_wait, 0

; fault handling API ---------------------------------------------------------

apifunc thread_get_pagefault
apifunc thread_get_miscfault
apifunc thread_get_zombie

; paging API -----------------------------------------------------------------

apifunc page_set
apifunc page_get
;apifunc pagetable_create
;apifunc pagetable_delete
;apifunc pagetable_switch
