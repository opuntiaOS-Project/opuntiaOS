.extern _start
.extern undefined_handler
.extern svc_handler
.extern prefetch_abort_handler
.extern data_abort_handler
.extern irq_handler
.extern fast_irq_handler

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
    ldmia   r0!,{r2, r3, r4, r5, r6, r7, r8, r9}
    stmia   r1!,{r2, r3, r4, r5, r6, r7, r8, r9}
    ldmia   r0!,{r2, r3, r4, r5, r6, r7, r8}
    stmia   r1!,{r2, r3, r4, r5, r6, r7, r8}
    pop     {r4, r5, r6, r7, r8, r9}

    ldr     r0, =_start
    add     r0, r0, #-1073741824 // Moving call to phys address (0xc00xxxxx -> 0x801xxxxx)
    add     r0, r0, #1048576
    mov     pc, r0

.section ".interrupt_vector_table"
.global trap_return
vector_table:
    b reset_handler
    b undefined_handler_isp
    b svc_isp
    b prefetch_abort_handler
    b data_abort_isp
    nop
    b irq_isp
vector_FIQ:
    b fast_irq_handler

irq_isp:
    subs    lr, lr, #4
    // Moving to SVC mode, to use it's stack.
    stmfd   sp!, {r1-r5}
    mrs     r1, spsr
    mrs     r2, sp_usr
    mrs     r3, lr_usr
    mov     r4, sp
    add     sp, sp, #20
    mov     r5, lr
    cps	    #0x13
   
    mov     lr, r5
    stmfd   sp!, {r6-r12,lr}

    mov     r6, r0
    ldr     r7, [r4]
    add     r4, r4, #4
    ldr     r8, [r4]
    add     r4, r4, #4
    ldr     r9, [r4]
    add     r4, r4, #4
    ldr     r10, [r4]
    add     r4, r4, #4
    ldr     r11, [r4]

    stmfd   sp!, {r6-r11}
    stmfd   sp!, {r1-r3}

    mov     r0, sp
    bl      irq_handler

    ldmfd   sp!, {r0-r2}
    msr     spsr, r0
    msr     sp_usr, r1
    msr     lr_usr, r2
    
    ldmfd   sp!, {r0-r12,lr}
    subs    pc, lr, #0
    nop

svc_isp:
    stmfd   sp!, {r0-r12,lr}
    mrs     r0, spsr
    mrs     r1, sp_usr
    mrs     r2, lr_usr
    stmfd   sp!, {r0-r2}

    mov     r0, sp
    bl      svc_handler

trap_return:
    ldmfd   sp!, {r0-r2}
    msr     spsr, r0
    msr     sp_usr, r1
    msr     lr_usr, r2
    
    ldmfd   sp!, {r0-r12,lr}
    subs    pc, lr, #0
    nop

undefined_handler_isp:
    subs    lr, lr, #4
    stmfd   sp!, {r0-r12,lr}
    mrs     r0, spsr
    mrs     r1, sp_usr
    mrs     r2, lr_usr
    stmfd   sp!, {r0-r2}

    mov     r0, sp
	bl      undefined_handler

	ldmfd   sp!, {r0-r2}
    msr     spsr, r0
    msr     sp_usr, r1
    msr     lr_usr, r2

    ldmfd   sp!, {r0-r12,lr}
    subs    pc, lr, #0
    nop

data_abort_isp:
    subs    lr, lr, #8
    stmfd   sp!, {r0-r12,lr}
    mrs     r0, spsr
    mrs     r1, sp_usr
    mrs     r2, lr_usr
    stmfd   sp!, {r0-r2}

    mov     r0, sp
	bl      data_abort_handler

	ldmfd   sp!, {r0-r2}
    msr     spsr, r0
    msr     sp_usr, r1
    msr     lr_usr, r2
    
    ldmfd   sp!, {r0-r12,lr}
    subs    pc, lr, #0
    nop

halt_cpu:
    wfi // wait for interrupt coming
    b halt_cpu

.global init_reset_handler
init_reset_handler:
    ldr sp, =STACK_TOP
reset_handler:
    mrc p15, #0, r0, c0, c0, #5 // Move to ARM Register r0 from Coprocessor c0. Read ID Code Register
    and r0, r0, #3 // r0 &= 0x3
    cmp r0, #0 // check whether r0==0
    beq _reset_cpu0 // reset cpu0

    cmp r0, #1 // check whether r0==1
    beq _reset_cpu1 // reset cpu1

    cmp r0, #2 // check whether r0==2
    beq _reset_cpu2 // reset cpu2

    cmp r0, #3 // check whether r0==3
    beq _reset_cpu3 // reset cpu3

    ldr pc,=halt_cpu

_reset_cpu0:
    push    {r4, r5, r6, r7, r8, r9}

    ldr     r0, =vector_table

    // set vector address.
    mcr P15, 0, r0, c12, c0, 0

    mov     r1, #0x0000
    ldmia   r0!,{r2, r3, r4, r5, r6, r7, r8, r9}
    stmia   r1!,{r2, r3, r4, r5, r6, r7, r8, r9}
    ldmia   r0!,{r2, r3, r4, r5, r6, r7, r8}
    stmia   r1!,{r2, r3, r4, r5, r6, r7, r8}
    pop     {r4, r5, r6, r7, r8, r9}
    ldr     pc, =_start

_reset_cpu1:
    push    {r4, r5, r6, r7, r8, r9}

    ldr     r0, =vector_table

    // set vector address.
    mcr P15, 0, r0, c12, c0, 0

    mov     r1, #0x0000
    ldmia   r0!,{r2, r3, r4, r5, r6, r7, r8, r9}
    stmia   r1!,{r2, r3, r4, r5, r6, r7, r8, r9}
    ldmia   r0!,{r2, r3, r4, r5, r6, r7, r8}
    stmia   r1!,{r2, r3, r4, r5, r6, r7, r8}
    pop     {r4, r5, r6, r7, r8, r9}
    ldr     pc, =_start

_reset_cpu2:
    push    {r4, r5, r6, r7, r8, r9}

    ldr     r0, =vector_table

    // set vector address.
    mcr P15, 0, r0, c12, c0, 0

    mov     r1, #0x0000
    ldmia   r0!,{r2, r3, r4, r5, r6, r7, r8, r9}
    stmia   r1!,{r2, r3, r4, r5, r6, r7, r8, r9}
    ldmia   r0!,{r2, r3, r4, r5, r6, r7, r8}
    stmia   r1!,{r2, r3, r4, r5, r6, r7, r8}
    pop     {r4, r5, r6, r7, r8, r9}
    ldr     pc, =_start

_reset_cpu3:
    push    {r4, r5, r6, r7, r8, r9}

    ldr     r0, =vector_table

    // set vector address.
    mcr P15, 0, r0, c12, c0, 0

    mov     r1, #0x0000
    ldmia   r0!,{r2, r3, r4, r5, r6, r7, r8, r9}
    stmia   r1!,{r2, r3, r4, r5, r6, r7, r8, r9}
    ldmia   r0!,{r2, r3, r4, r5, r6, r7, r8}
    stmia   r1!,{r2, r3, r4, r5, r6, r7, r8}
    pop     {r4, r5, r6, r7, r8, r9}
    ldr     pc, =_start

.section ".text"
.global swi
swi:
    push {lr}
    swi 0x0
    pop {pc}

.global set_svc_stack
set_svc_stack:
    mov     r1, sp
    mov     r2, lr
    cps	    #0x13       /* set svc mode */
    mov	    sp, r0
    cps	    #0x1F		/* set system mode */
    mov     sp, r1
    bx      r2

.global set_irq_stack
set_irq_stack:
    mov     r1, sp
    mov     r2, lr
    cps	    #0x12       /* set irq mode */
    mov	    sp, r0
    cps	    #0x1F		/* set system mode */
    mov     sp, r1
    bx      r2

.global set_abort_stack
set_abort_stack:
    mov     r1, sp
    mov     r2, lr
    cps	    #0x17       /* set abort mode */
    mov	    sp, r0
    cps	    #0x1F		/* set system mode */
    mov     sp, r1
    bx      r2

.global set_undefined_stack
set_undefined_stack:
    mov     r1, sp
    mov     r2, lr
    cps	    #0x1B       /* set abort mode */
    mov	    sp, r0
    cps	    #0x1F		/* set system mode */
    mov     sp, r1
    bx      r2
