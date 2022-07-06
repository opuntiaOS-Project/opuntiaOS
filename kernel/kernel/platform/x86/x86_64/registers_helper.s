[bits 64]

global read_ip
read_ip:
    mov rax, [rsp]
    ret