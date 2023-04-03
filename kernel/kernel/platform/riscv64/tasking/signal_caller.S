.global signal_caller_start
.global signal_caller_end

signal_caller_start:
    jalr a1 // calling signal handler
    mv a1, a0
    li a7, 119 // sigreturn syscall
    ecall
signal_caller_end:
