[bits 32]
[extern main]
push dword [esp+4]
call main
jmp $