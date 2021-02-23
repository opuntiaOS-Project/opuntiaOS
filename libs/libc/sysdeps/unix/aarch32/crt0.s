.section .text

.extern main
.extern exit
.extern _init

.global _start
_start:
	bl _init
	bl main
	bl exit
