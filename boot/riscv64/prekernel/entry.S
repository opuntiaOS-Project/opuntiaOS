.extern main
.extern main_s_mode

.section ".opuntiaos_boot_text", "xa"
prekernel_header:
    auipc a0, 0
    la a1, PREKERNEL_STACK_TOP_OFFSET
    mv sp, a1
    j main

.global enable_mmu
enable_mmu:
    fence rw, rw
    csrw satp, a0
    csrs sstatus, a1
    sfence.vma
    ret

.global jump_to_kernel
jump_to_kernel:
    fence rw, rw
    sfence.vma
    jr a1
loop:
    j loop

.align 4
timervec:
    csrrw a0, mscratch, a0
    sd a1, 0(a0)
    sd a2, 8(a0)
    sd a3, 16(a0)

    ld a1, 24(a0)
    ld a2, 32(a0)
    ld a3, 0(a1)
    add a3, a3, a2
    sd a3, 0(a1)

    li a1, 2
    csrw sip, a1

    ld a3, 16(a0)
    ld a2, 8(a0)
    ld a1, 0(a0)
    csrrw a0, mscratch, a0

    mret

.global setup_timer_m_mode
setup_timer_m_mode:
    csrw mscratch, a0
    la t0, timervec
    csrw mtvec, t0

    csrr t0, mstatus // Add flag MSTATUS_MIE | MSTATUS_SUM
    li t1, (1 << 3) | (1 << 18)
    or t0, t0, t1
    csrw mstatus, t0

    csrr t0, mie // Add flag MIE_MTIE
    li t1, (1 << 7)
    or t0, t0, t1
    csrw mie, t0

    ret

.global jump_to_s_mode
jump_to_s_mode:
    csrr t0, mstatus
    li t1, 3 << 11
    and t0, t0, t1
    li t1, 1 << 11
    or t0, t0, t1
    csrw mstatus, t0

    la t1, sv_main
	csrw mepc, t1

    csrw satp, zero

    li t1, 0xffff
    csrw medeleg, t1
    csrw mideleg, t1

    li t1, (1 << 1) | (1 << 5) | (1 << 9)
    csrr t0, sie
    or t0, t0, t1
    csrw sie, t0

    li t1, 0x3fffffffffffff
    csrw pmpaddr0, t1

    li t1, 0xf
    csrw pmpcfg0, t1

    mret

sv_main:
    j main_s_mode