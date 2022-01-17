section .opuntiaos_kernel_boot

[bits 32]
extern stage3
global _start
_start:
    push dword [esp+4]
    call stage3
    jmp $