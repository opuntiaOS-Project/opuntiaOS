section .text
; push ebp
mov esp, ebp
mov ebx, 332
int 50 ; fork
int 48 ; printing result
start:
    mov eax, 0x3
    ; int 48
    jmp start