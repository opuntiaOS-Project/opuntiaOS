.section .text

.extern main
.extern exit
.extern _init
.extern _deinit

.global _start
_start:
	sub sp, sp, #0x40
	stp x0,x1,[sp]
	stp x2,x3,[sp,#0x10]
	bl      _init
	ldp x0,x1,[sp]
	ldp x2,x3,[sp,#0x10]
	bl      main
	stp x0,x1,[sp]
	bl      _deinit
	ldp x0,x1,[sp]
	add sp, sp, #0x40
	bl      exit
