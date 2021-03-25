.global fpu_save
fpu_save:
    vstm    r0!, {d0-d15}
    vstm    r0!, {d16-d31}
    bx      lr

.global fpu_restore
fpu_restore:
    vldm    r0!, {d0-d15}
    vldm    r0!, {d16-d31}
    bx      lr

.global read_fpexc
read_fpexc:
    vmrs    r0, fpexc
    isb
    bx      lr

.global write_fpexc
write_fpexc:
    vmsr    fpexc, r0
    isb
    bx      lr
