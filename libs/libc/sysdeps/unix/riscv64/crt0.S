.section .text

.extern main
.extern exit
.extern _init
.extern _deinit
.extern __global_pointer$

.global _start
_start:
	.option push
    .option norelax
    la gp, __global_pointer$
    .option pop
	addi sp, sp, -0x40
	sd a0, 0(sp)
	sd a1, 8(sp)
	sd a2, 16(sp)
	sd a3, 24(sp)
	call _init
	ld a0, 0(sp)
	ld a1, 8(sp)
	ld a2, 16(sp)
	ld a3, 24(sp)
	call main
	sd a0, 0(sp)
	sd a1, 8(sp)
	call _deinit
	ld a0, 0(sp)
	ld a1, 8(sp)
	addi sp, sp, 0x40
	call exit
