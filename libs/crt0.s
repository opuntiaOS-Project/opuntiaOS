section .text

extern main
extern exit

global _start:function (_start.end - _start)
_start:
	call main
	push eax
	call exit
.end: