.global setjmp
setjmp:
    stp x8, x9, [x0,#0x0]
    stp x10, x11, [x0,#0x10]
    stp x12, x13, [x0,#0x20]
    stp x14, x15, [x0,#0x30]
    stp x16, x17, [x0,#0x40]
    stp x18, x19, [x0,#0x50]
    stp x20, x21, [x0,#0x60]
    stp x22, x23, [x0,#0x70]
    stp x24, x25, [x0,#0x80]
    stp x26, x27, [x0,#0x90]
    stp x28, x29, [x0,#0xa0]
    mov x2, lr
    mov x3, sp
    stp x2, x3, [x0,#0xb0]
    // TODO(aarch64): Add fpu support.
    mov	x0, #0
	ret

.global longjmp
longjmp:
    ldp x8, x9, [x0,#0x0]
    ldp x10, x11, [x0,#0x10]
    ldp x12, x13, [x0,#0x20]
    ldp x14, x15, [x0,#0x30]
    ldp x16, x17, [x0,#0x40]
    ldp x18, x19, [x0,#0x50]
    ldp x20, x21, [x0,#0x60]
    ldp x22, x23, [x0,#0x70]
    ldp x24, x25, [x0,#0x80]
    ldp x26, x27, [x0,#0x90]
    ldp x28, x29, [x0,#0xa0]
    ldp x2, x3, [x0,#0xb0]
    mov lr, x2
    mov sp, x3
    mov x0, x1
    cmp x0, #0
    bne 1f
    mov	x0, #1
1:
    ret
