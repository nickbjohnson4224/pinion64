[bits 64]

; exported symbols
global pmm_init

global pmm_new4096
global pmm_new2048
global pmm_new1024
global pmm_new512
global pmm_new256

global pmm_del4096
global pmm_del2048
global pmm_del1024
global pmm_del512
global pmm_del256

; imported symbols

section .text

pmm_init:

	ret
