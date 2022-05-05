.section ".text"
.global set_up_vector_table
set_up_vector_table:
    ldr x0, =vector_table
    msr vbar_el1, x0
    isb
    ret


.align 12
.global vector_table
vector_table:
    // Current EL with SP0
    msr spsel, #1
    sub sp, sp, #0x120
    stp x0,x1,[sp]
    msr spsel, #0
    mov x1, sp
    msr spsel, #1
    adr x0, sync_exc_jmp
    b exc_handler // Synchronous
.balign 128
    msr spsel, #1
    sub sp, sp, #0x120
    stp x0,x1,[sp]
    msr spsel, #0
    mov x1, sp
    msr spsel, #1
    adr x0, irq_exc_jmp
    b exc_handler // IRQ/vIRQ
.balign 128
    msr spsel, #1
    sub sp, sp, #0x120
    stp x0,x1,[sp]
    msr spsel, #0
    mov x1, sp
    msr spsel, #1
    adr x0, fiq_exc_jmp
    b exc_handler // FIQ/vFIQ
.balign 128
    msr spsel, #1
    sub sp, sp, #0x120
    stp x0,x1,[sp]
    msr spsel, #0
    mov x1, sp
    msr spsel, #1
    adr x0, serror_exc_jmp
    b exc_handler // SError/vSError

    // Current EL with SPn
.balign 128
    msr spsel, #1
    sub sp, sp, #0x120
    stp x0,x1,[sp]
    add x1, sp, #0x120
    adr x0, sync_exc_jmp
    b exc_handler // Synchronous
.balign 128
    msr spsel, #1
    sub sp, sp, #0x120
    stp x0,x1,[sp]
    add x1, sp, #0x120
    adr x0, irq_exc_jmp
    b exc_handler // IRQ/vIRQ
.balign 128
    msr spsel, #1
    sub sp, sp, #0x120
    stp x0,x1,[sp]
    add x1, sp, #0x120
    adr x0, fiq_exc_jmp
    b exc_handler // FIQ/vFIQ
.balign 128
    msr spsel, #1
    sub sp, sp, #0x120
    stp x0,x1,[sp]
    add x1, sp, #0x120
    adr x0, serror_exc_jmp
    b exc_handler // SError/vSError

    // Lower EL with Aarch64
.balign 128
    msr spsel, #1
    sub sp, sp, #0x120
    stp x0,x1,[sp]
    msr spsel, #0
    mov x1, sp
    msr spsel, #1
    adr x0, sync_exc_jmp
    b exc_handler // Synchronous
.balign 128
    msr spsel, #1
    sub sp, sp, #0x120
    stp x0,x1,[sp]
    msr spsel, #0
    mov x1, sp
    msr spsel, #1
    adr x0, irq_exc_jmp
    b exc_handler // IRQ/vIRQ
.balign 128
    msr spsel, #1
    sub sp, sp, #0x120
    stp x0,x1,[sp]
    msr spsel, #0
    mov x1, sp
    msr spsel, #1
    adr x0, fiq_exc_jmp
    b exc_handler // FIQ/vFIQ
.balign 128
    msr spsel, #1
    sub sp, sp, #0x120
    stp x0,x1,[sp]
    msr spsel, #0
    mov x1, sp
    msr spsel, #1
    adr x0, serror_exc_jmp
    b exc_handler // SError/vSError
    
    // Lower EL with Aarch32
.balign 128
    b . // Synchronous
.balign 128
    b .  // IRQ/vIRQ
.balign 128
    b .  // FIQ/vFIQ
.balign 128
    b . // SError/vSError



.extern sync_handler
.extern irq_handler
.extern fast_irq_handler
.extern serror_handler
sync_exc_jmp:
    ldr x1, =sync_handler
    br x1
irq_exc_jmp:
    ldr x1, =irq_handler
    br x1
fiq_exc_jmp:
    ldr x1, =fast_irq_handler
    br x1
serror_exc_jmp:
    ldr x1, =serror_handler
    br x1


exc_handler:
    isb
    dmb sy

    str x1, [sp,#0x118]
    stp x2,x3,[sp,#0x10]
    stp x4,x5,[sp,#0x20]
    stp x6,x7,[sp,#0x30]
    stp x8,x9,[sp,#0x40]
    stp x10,x11,[sp,#0x50]
    stp x12,x13,[sp,#0x60]
    stp x14,x15,[sp,#0x70]
    stp x16,x17,[sp,#0x80]
    stp x18,x19,[sp,#0x90]
    stp x20,x21,[sp,#0xa0]
    stp x22,x23,[sp,#0xb0]
    stp x24,x25,[sp,#0xc0]
    stp x26,x27,[sp,#0xd0]
    stp x28,x29,[sp,#0xe0]
    
    mrs x29, esr_el1
    stp x30,x29,[sp,#0xf0]
    mov x16, x0
    mrs x0, elr_el1
    mrs x1, far_el1
    stp x0,x1,[sp,#0x100]
    mrs x1, spsr_el1
    str x1, [sp, #0x110]

    mov x0, sp
    blr x16

.global trap_return
trap_return:
    ldp x0,x1,[sp,#0x100]
    msr elr_el1, x0
    msr far_el1, x1

    ldr x1, [sp, #0x110]
    msr spsr_el1, x1
    
    mov x0, sp
    
    ldp x2,x3,[x0,#0x10]
    ldp x4,x5,[x0,#0x20]
    ldp x6,x7,[x0,#0x30]
    ldp x8,x9,[x0,#0x40]
    ldp x10,x11,[x0,#0x50]
    ldp x12,x13,[x0,#0x60]
    ldp x14,x15,[x0,#0x70]
    ldp x16,x17,[x0,#0x80]
    ldp x18,x19,[x0,#0x90]
    ldp x20,x21,[x0,#0xa0]
    ldp x22,x23,[x0,#0xb0]
    ldp x24,x25,[x0,#0xc0]
    ldp x26,x27,[x0,#0xd0]
    ldp x28,x29,[x0,#0xe0]
    ldr x30,[x0,#0xf0]

    add sp, sp, #0x120

    // Switch back to correct spsel to restore SP upon exception return
    tbnz x1, #0, 1f
    msr spsel, #0
    b 2f
1:
    msr spsel, #1
2:
    ldr x1, [x0, #0x118]
    mov sp, x1
    ldp x0, x1, [x0]
    eret
    b .
