mov ebx, 9
loop:
    mov eax, 0x00 ; print
    int 0x80 ; syscall
    jmp loop