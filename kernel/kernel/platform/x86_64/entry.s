section .opuntiaos_kernel_boot

[bits 64]
; extern stage3
extern STACK_TOP
global _start
_start:
    mov rax, rsp
    mov rsp, STACK_TOP
    ; call stage3
    jmp $