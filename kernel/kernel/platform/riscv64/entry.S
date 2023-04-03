.extern stage3

.section ".opuntiaos_kernel_boot", "xa"
kernel_entry:
1:
    la a1, STACK_TOP
    mv sp, a1
    call stage3

park:
    wfi
    j park
