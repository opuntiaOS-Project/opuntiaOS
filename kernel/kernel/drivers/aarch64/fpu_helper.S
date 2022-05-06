.global fpu_save
fpu_save:
    stp d0, d1, [x0]
    stp d2, d3, [x0,#0x20]
    stp d4, d5, [x0,#0x40]
    stp d6, d7, [x0,#0x60]
    stp d8, d9, [x0,#0x80]
    stp d10, d11, [x0,#0xa0]
    stp d12, d13, [x0,#0xc0]
    stp d14, d15, [x0,#0xe0]
    stp d16, d17, [x0,#0x100]
    stp d18, d19, [x0,#0x120]
    stp d20, d21, [x0,#0x140]
    stp d22, d23, [x0,#0x160]
    stp d24, d25, [x0,#0x180]
    stp d26, d27, [x0,#0x1a0]
    stp d28, d29, [x0,#0x1c0]
    stp d30, d31, [x0,#0x1e0]
    dsb sy
    ret

.global fpu_restore
fpu_restore:
    ldp d0, d1, [x0]
    ldp d2, d3, [x0,#0x20]
    ldp d4, d5, [x0,#0x40]
    ldp d6, d7, [x0,#0x60]
    ldp d8, d9, [x0,#0x80]
    ldp d10, d11, [x0,#0xa0]
    ldp d12, d13, [x0,#0xc0]
    ldp d14, d15, [x0,#0xe0]
    ldp d16, d17, [x0,#0x100]
    ldp d18, d19, [x0,#0x120]
    ldp d20, d21, [x0,#0x140]
    ldp d22, d23, [x0,#0x160]
    ldp d24, d25, [x0,#0x180]
    ldp d26, d27, [x0,#0x1a0]
    ldp d28, d29, [x0,#0x1c0]
    ldp d30, d31, [x0,#0x1e0]
    dsb sy
    ret