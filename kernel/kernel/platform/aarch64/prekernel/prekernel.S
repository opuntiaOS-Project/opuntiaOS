.extern stage3
#ifdef TARGET_QEMU_VIRT
.extern qemu_virt_boot_init
.extern vm_setup
#endif

.section ".prekernel_entry"
// Prekernel works with start addresses.
prekernel_entry:
    ldr x30, =PREKERNEL_STACK_TOP
    mov sp, x30
#ifdef TARGET_QEMU_VIRT
    ldr x3, =qemu_virt_boot_init
    blr x3
#endif
    mov x19, x0
    ldr x3, =vm_setup
    blr x3
    mov x0, x19
    ldr x30, =STACK_TOP
    mov sp, x30
    ldr x3, =stage3
    blr x3
entry_loop:
    b entry_loop

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