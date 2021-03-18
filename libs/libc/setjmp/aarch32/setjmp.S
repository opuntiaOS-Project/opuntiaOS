.global setjmp
setjmp:
    stm     r0!, {r4-r12,sp,lr}
#ifdef __ARM_NEON__
    vstm    r0!, {d8-d15}
#endif // __ARM_NEON__
    mov	    r0, #0
	bx      lr

.global longjmp
longjmp:
    ldm    r0!, {r4-r12,sp,lr}
#ifdef __ARM_NEON__
    vldm    r0!, {d8-d15}
#endif // __ARM_NEON__
    mov     r0, r1
    cmp     r0, #0
    bne     1f
    mov	    r0, #1
1:
    bx      lr
