.global signal_caller_start
.global signal_caller_end

signal_caller_start:
    blx     r1 // calling signal handler
    mov     r1, r0
    mov     r7, #119 // sigreturn syscall
    swi     0x0
signal_caller_end:
