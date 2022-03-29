.section .text

.extern main
.extern exit
.extern _init
.extern _deinit

.global _start
_start:
	push    {r0-r2}
	sub     sp, sp, #4 // align to 16bytes
	bl      _init
	add     sp, sp, #4
	pop     {r0-r2}
	bl      main
	push    {r0}
	sub     sp, sp, #12 // align to 16bytes
	bl      _deinit
	add     sp, sp, #12
	pop     {r0}
	bl      exit
