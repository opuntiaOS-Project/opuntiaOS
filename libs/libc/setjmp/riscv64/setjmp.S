.global setjmp
setjmp:
    sd s0, 0(a0)
    sd s1, 8(a0)
    sd s2, 16(a0)
    sd s3, 24(a0)
    sd s4, 32(a0)
    sd s5, 40(a0)
    sd s6, 48(a0)
    sd s7, 56(a0)
    sd s8, 64(a0)
    sd s9, 72(a0)
    sd s10, 80(a0)
    sd s11, 88(a0)
    sd ra, 96(a0)
    sd sp, 104(a0)

    mv a0, zero
    ret
    // stp x19, x20, [x0,#0x0]
    // stp x21, x22, [x0,#0x10]
    // stp x23, x24, [x0,#0x20]
    // stp x25, x26, [x0,#0x30]
    // stp x27, x28, [x0,#0x40]
    // stp x29, x4, [x0,#0x50]
    // mov x2, lr
    // mov x3, sp
    // stp x2, x3, [x0,#0x60]
    // stp d8, d9, [x0,#0x70]
    // stp d10, d11, [x0,#0x80]
    // stp d12, d13, [x0,#0x90]
    // stp d14, d15, [x0,#0xa0]
    // mov	x0, #0
	ret

.global longjmp
longjmp:
    ld s0, 0(a0)
    ld s1, 8(a0)
    ld s2, 16(a0)
    ld s3, 24(a0)
    ld s4, 32(a0)
    ld s5, 40(a0)
    ld s6, 48(a0)
    ld s7, 56(a0)
    ld s8, 64(a0)
    ld s9, 72(a0)
    ld s10, 80(a0)
    ld s11, 88(a0)
    ld ra, 96(a0)
    ld sp, 104(a0)
    bne a0, zero, 1f
    li	a0, 1
1:
    ret
