global signal_caller_start
global signal_caller_end

signal_caller_start:
    push ebp
    mov ebp, esp
    mov eax, [ebp+8] ; signal code
    mov ebx, [ebp+12] ; handler
    push eax ; pushing signal code
    call ebx ; calling signal handler
    add esp, 8 ; cleaning stack
    mov ebx, eax
    mov eax, 10 ; sigreturn syscall
    int 0x80
signal_caller_end: