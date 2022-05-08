.global setjmp
setjmp:
    stp x19, x20, [x0,#0x0]
    stp x21, x22, [x0,#0x10]
    stp x23, x24, [x0,#0x20]
    stp x25, x26, [x0,#0x30]
    stp x27, x28, [x0,#0x40]
    stp x29, x4, [x0,#0x50]
    mov x2, lr
    mov x3, sp
    stp x2, x3, [x0,#0x60]
    stp d8, d9, [x0,#0x70]
    stp d10, d11, [x0,#0x80]
    stp d12, d13, [x0,#0x90]
    stp d14, d15, [x0,#0xa0]
    mov	x0, #0
	ret

.global longjmp
longjmp:
    ldp x19, x20, [x0,#0x0]
    ldp x21, x22, [x0,#0x10]
    ldp x23, x24, [x0,#0x20]
    ldp x25, x26, [x0,#0x30]
    ldp x27, x28, [x0,#0x40]
    ldp x29, x4, [x0,#0x50]
    ldp x2, x3, [x0,#0x60]
    ldp d8, d9, [x0,#0x70]
    ldp d10, d11, [x0,#0x80]
    ldp d12, d13, [x0,#0x90]
    ldp d14, d15, [x0,#0xa0]
    mov lr, x2
    mov sp, x3
    mov x0, x1
    cmp x0, #0
    bne 1f
    mov	x0, #1
1:
    ret
