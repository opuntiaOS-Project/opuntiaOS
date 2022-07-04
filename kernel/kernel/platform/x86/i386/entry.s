section .opuntiaos_kernel_boot

[bits 32]
extern stage3
extern STACK_TOP
global _start
_start:
    mov eax, esp
    mov esp, STACK_TOP
    push dword [eax+4]
    call stage3
    jmp $