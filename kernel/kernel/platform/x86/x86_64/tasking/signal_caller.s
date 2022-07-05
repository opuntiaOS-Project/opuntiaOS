[bits 64]

global signal_caller_start
global signal_caller_end

signal_caller_start:
    call rsi
    mov rdi, rax
    mov rax, 119 ; sigreturn syscall
    int 0x80
signal_caller_end: