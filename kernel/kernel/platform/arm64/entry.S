.extern stage3

.section ".opuntiaos_kernel_boot"
kernel_entry:
    msr spsel, #1
    ldr x30, =STACK_TOP
    mov sp, x30
    ldr x3, =stage3
    blr x3
entry_loop:
    b entry_loop