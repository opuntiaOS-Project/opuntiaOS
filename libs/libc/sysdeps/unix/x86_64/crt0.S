[bits 64]

section .text

extern main
extern exit
extern _init
extern _deinit

global _start:function (_start.end - _start)
_start:
	push rsi
	push rdi
	push rdx
	push rcx
	call _init
	pop rcx
	pop rdx
	pop rdi
	pop rsi
	call main
	push rax
	call _deinit
	pop rax
	mov rdi, rax
	call exit
.end: