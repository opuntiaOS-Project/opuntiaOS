.extern _start
.extern vector_table

.section ".oneos_load_section"
oneos_loader:
    ldr     sp, =STACK_PHYZ_TOP

    // The same as reset cpu 0
    mrc     p15, #0, r0, c0, c0, #5 // Move to ARM Register r0 from Coprocessor c0. Read ID Code Register
    and     r0, r0, #3 // r0 &= 0x3
    push    {r4, r5, r6, r7, r8, r9}

    ldr     r0, =vector_table // Note, that we loading virtual address already here.

    // set vector address.
    mcr     P15, 0, r0, c12, c0, 0

    mov     r1, #0x0000
    ldmia   r0!, {r2, r3, r4, r5, r6, r7, r8, r9}
    stmia   r1!, {r2, r3, r4, r5, r6, r7, r8, r9}
    ldmia   r0!, {r2, r3, r4, r5, r6, r7, r8}
    stmia   r1!, {r2, r3, r4, r5, r6, r7, r8}
    pop     {r4, r5, r6, r7, r8, r9}

    ldr     r0, =_start
    add     r0, r0, #-1073741824 // Moving call to phys address (0xc00xxxxx -> 0x801xxxxx)
    add     r0, r0, #1048576
    mov     pc, r0
