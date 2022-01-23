section .opuntiaos_boot_text

[bits 32]
extern stage2
extern STACK_PHYZ_TOP
global _start
_start:
    mov eax, esp
    mov esp, STACK_PHYZ_TOP
    push dword [eax+4]
    call stage2
    jmp $