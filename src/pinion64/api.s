[bits 64]

%macro apifunc 1
	global pinion_%1:function default
	extern apifunc_%1

	section .text

	pinion_%1:
		cli
		swapgs
		mov r11, rsp
		mov rsp, [gs:0x10]
		push r11
		call apifunc_%1
		pop rsp
		swapgs
		sti
		ret

%endmacro

; thread API
apifunc thread_yield
