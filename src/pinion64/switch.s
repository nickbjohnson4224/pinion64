[bits 64]

; exported symbols
global pcx_switch:function internal

; imported symbols
extern pmm_base

pcx_switch:
	
	mov rax, [rel pmm_base]
	sub rdi, rax
	mov cr3, rdi
	ret
