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
