[bits 64]

; exported symbols
global mutex_trylock:function internal
global mutex_acquire:function internal
global mutex_release:function internal

section .text

	mutex_trylock:

		mov cl, 0x01
		mov al, [rdi]
		test al, al
		jnz .fail

		lock cmpxchg [rdi], cl

		test al, al
		jnz .fail
		mov rax, 1
		ret

		.fail:
		xor rax, rax
		ret

	mutex_acquire:

		mov cl, 0x01

		.spin:
		mov al, [rdi]
		test al, al
		jnz .spin

		lock cmpxchg [rdi], cl
		
		test al, al
		jnz .spin

		ret

	mutex_release:

		xor al, al
		mov [rdi], al
		ret
