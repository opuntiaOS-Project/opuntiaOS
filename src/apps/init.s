section .text
; push ebp
mov esp, ebp
mov ebx, 332
start:
    mov eax, 0x3
    int 48
    jmp start