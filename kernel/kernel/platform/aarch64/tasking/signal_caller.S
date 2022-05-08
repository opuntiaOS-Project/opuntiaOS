.global signal_caller_start
.global signal_caller_end

signal_caller_start:
    blr     x1 // calling signal handler
    mov     x1, x0
    mov     x8, #119 // sigreturn syscall
    svc     0x0
signal_caller_end:
