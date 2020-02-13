section .text
mov ebx, 332
int 48
start:
    mov eax, 1
    jmp start
