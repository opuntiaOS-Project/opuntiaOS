.extern _start
.extern _start_secondary_cpu
@ .extern vector_table

.section ".opuntiaos_load_section"
opuntiaos_loader:
    ldr     sp, =STACK_PHYZ_TOP

    mrc     p15, #0, r0, c0, c0, #5 // Read CPU ID register.
    and     r0, r0, #3
    cmp     r0, #0
    bne     secondary_cpu_loader

    ldr     r0, =_start
    mov     pc, r0

secondary_cpu_loader:
    ldr     sp, =STACK_SECONDARY_PHYZ_TOP
    mov     r9, #512
    mrc     p15, #0, r8, c0, c0, #5 // Read CPU ID register.
    and     r8, r8, #3
1:
    sub     r8, r8, #1
    cmp     r8, #0
    beq     secondary_cpu_loader_exit
    add     r9, r9, #512
    b       1b
secondary_cpu_loader_exit:
    subs    sp, r9
    ldr     r0, =_start_secondary_cpu
    mov     pc, r0
