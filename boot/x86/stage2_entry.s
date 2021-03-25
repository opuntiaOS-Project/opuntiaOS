[bits 32]
[extern stage2]
global _start
_start:
    push dword [esp+4]
    call stage2
    jmp $