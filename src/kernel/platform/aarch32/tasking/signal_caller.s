.global signal_caller_start
.global signal_caller_end

// FIXME
signal_caller_start:
    mov     r0, #10
    bx      r1 // calling signal handler
    //mov     r0, eax
    mov     r7, #14 // sigreturn syscall
    swi     0x0
signal_caller_end:
