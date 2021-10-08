.section .text

.global main

main:
    mov r7, #11
    ldr r0, =msg
    swi 11
loop:
    b loop
.data
.align
msg: .asciz "/System/initsystem"
