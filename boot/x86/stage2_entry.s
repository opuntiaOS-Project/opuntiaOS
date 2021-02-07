[bits 32]
[extern stage2]
push dword [esp+4]
call stage2
jmp $