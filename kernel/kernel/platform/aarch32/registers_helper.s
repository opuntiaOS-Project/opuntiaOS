.global read_bp
.global read_ip

read_bp:
    mov r0, r11
    bx lr

read_ip:
    mov r0, lr
    bx lr
