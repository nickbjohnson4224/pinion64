[bits 64]

section .text

global outb

outb:
	mov ax, si
	mov dx, di
	out dx, al
	ret

global inb

inb:
	mov dx, di
	xor rax, rax
	in  al, dx
	ret
