.section .text

.extern main
.extern exit
.extern _init
.extern _deinit

.global _start
_start:
	push {r0-r2}
	bl _init
	pop {r0-r2}
	bl main
	push {r0}
	bl _deinit
	pop {r0}
	bl exit
