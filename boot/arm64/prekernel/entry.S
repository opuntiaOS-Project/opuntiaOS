.extern main

.section ".opuntiaos_boot_text"
prekernel_header:
    nop // TODO(arm64): Add UEFI support
    b prekernel_entry

prekernel_entry:
    // Calculating location where of start address.
    adr x20, .
    adr x22, prekernel_header
    adr x23, prekernel_entry
    sub x21, x23, x22
    sub x20, x20, x21

    // Saving devtree
    mov x21, x0

    // Setting up prekernel stack.
    ldr x30, =PREKERNEL_STACK_TOP_OFFSET
    add x30, x20, x30
    mov sp, x30

    // Base of kernel
    mov x0, x20
    // Header location
    ldr x1, =PREKERNEL_END_OFFSET
    add x1, x20, x1
    // Devtree location
    mov x2, x21
    ldr x3, =main
    add x3, x3, x20
    blr x3
entry_loop:
    b entry_loop

.global jump_to_kernel
jump_to_kernel:
    ic iallu
    dsb sy
    isb sy
    blr x1
jump_loop:
    b jump_loop

.global enable_mmu_el1
enable_mmu_el1:
    dsb sy
    msr mair_el1, x2
    msr tcr_el1, x1
    msr ttbr0_el1, x0
    msr ttbr1_el1, x3
    isb sy
    tlbi vmalle1
    isb sy
    ic iallu
    isb sy
    mrs x3, sctlr_el1
    orr x3, x3, #1
    orr x3, x3, #4
    and x3, x3, #(~2)
    msr sctlr_el1, x3
    ic iallu
    dsb sy
    isb sy
    ret