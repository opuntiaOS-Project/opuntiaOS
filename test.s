main:
    mov eax, 3
    mov [data], eax
    mov ebx, [data]
    int 0x01
    ret
data: db 0