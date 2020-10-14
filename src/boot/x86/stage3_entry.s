section .oneos_kernel_boot

[bits 32]
[extern stage3]
push dword [esp+4]
call stage3
jmp $