section .text
; push ebp
mov esp, ebp
mov ebx, 332
int 50 ; fork
int 48 ; printing eax
cmp eax, 0
jne ffe
ffe2:
    mov edx, 0x233
    mov [edx], eax
start:
    int 48
    jmp start
ffe:
    int 50 ; fork
    int 48 ; printing eax
    jmp ffe2