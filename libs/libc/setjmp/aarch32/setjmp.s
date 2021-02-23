.global setjmp
setjmp:
    add	    r0, r0, #44
    stmfd   r0!, {r4-r12,sp,lr}
    mov	    r0, #0
	bx      lr

.global longjmp
longjmp:
    ldmfd   r0!, {r4-r12,sp,lr}
    mov     r0, r1
    cmp     r0, #0
    bne     1f
    mov	    r0, #1
1:
    bx      lr
