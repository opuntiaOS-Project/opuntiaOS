.extern arm_mem_desc
.extern vm_setup
.extern stage3
.global _start
_start:
    ldr     sp, =STACK_PHYZ_TOP
    ldr     r0, =vm_setup
    ldr     lr, =_start_return
    mov     pc, r0
_start_return:
    // After enabling VM, we need to recalc all addresses to their new base.
    // We are adding 0x40000000 to move everything to 3gb mark
    ldr     sp, =STACK_TOP
    ldr     r1, =.arm_mem_desc_addr
    ldr     r0, [r1]
    bl stage3
1:
    b 1b
.size _start, . - _start

.arm_mem_desc_addr:
    .word arm_mem_desc
