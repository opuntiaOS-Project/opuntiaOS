[bits 32]
[extern stage3]
push dword [esp+4]
call stage3
jmp $