/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Target ARMv7.

.global memset

// r0 - dest
// r1 - ch
// r2 - len
memset:
    mov     r3, r0
    
    cmp     r2, #0
    beq     memset_exit

    tst     r3, #3 // Will set N 
    beq     memset_4bytes_aligned_entry

memset_byte:
    subs	r2, r2, #1
    strb    r1, [r3], #1
    beq     memset_exit
    
    tst     r3, #3
    bne     memset_byte

memset_4bytes_aligned_entry:
    cmp     r2, #4
    blt     memset_byte_2
    
    and	    r1, #0xFF
	orr	    r1, r1, r1, lsl#8
	orr	    r1, r1, r1, lsl#16
    
    tst     r3, #15
    beq     memset_16bytes_aligned_entry

memset_4bytes_aligned_loop:
    subs	r2, r2, #4
    str     r1, [r3], #4
    
    cmp     r2, #0
    beq     memset_exit
    
    cmp     r2, #4
    blt     memset_byte_2

    tst     r3, #15 // Test if we can put 16bytes block.
    bne     memset_4bytes_aligned_loop // If not repeat 4 byte block again.

memset_16bytes_aligned_entry:
    push    {r5, r6, r7}
    cmp     r2, #16
    blt     memset_16bytes_aligned_exit

memset_16bytes_aligned_preloop:
    // Load ch into all bytes of 32bit r1 register
    mov     r5, r1
    mov     r6, r1
    mov     r7, r1

memset_16bytes_aligned_loop:
    subs	r2, r2, #16
    stmia	r3!,{r1,r5,r6,r7}
    
    cmp     r2, #16
    bge     memset_16bytes_aligned_loop

memset_16bytes_aligned_exit:
    pop     {r5, r6, r7}
    cmp     r2, #0
    beq     memset_exit

memset_4bytes_aligned_entry_2:
    cmp     r2, #4
    blt     memset_byte_2

memset_4bytes_aligned_loop_2:
    subs	r2, r2, #4
    str     r1, [r3], #4
    
    cmp     r2, #0
    beq     memset_exit
    
    cmp     r2, #4
    bge     memset_4bytes_aligned_loop_2

memset_byte_2:
    subs	r2, r2, #1
    strb    r1, [r3], #1
    bne     memset_byte_2

memset_exit:
    bx lr
