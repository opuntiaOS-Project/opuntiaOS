[bits 64]

global setjmp
setjmp:
    mov [rdi], rbx
    mov [rdi+8], r12
    mov [rdi+16], r13
    mov [rdi+24], r14
    mov [rdi+32], r15
    mov [rdi+40], rbp
    mov [rdi+48], rsp

    mov rax, [rsp]
    mov [rdi+56], rax

    xor rax, rax
    ret

global longjmp
longjmp:
    mov rbx, [rdi]
    mov r12, [rdi+8]
    mov r13, [rdi+16]
    mov r14, [rdi+24]
    mov r15, [rdi+32]
    mov rbp, [rdi+40]
    mov rsp, [rdi+48]
    mov rcx, [rdi+56]

    mov rax, rsi
    test rax, rax
    jnz success
    mov rax, 1    
success:
    jmp rcx