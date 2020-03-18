section .text
; push ebp
mov esp, ebp
mov ebx, 332
int 50 ; fork
int 48 ; printing eax
mov eax, 0x233
mov [eax], word 3
start:
    mov eax, 0x3
    ; int 48
    jmp start