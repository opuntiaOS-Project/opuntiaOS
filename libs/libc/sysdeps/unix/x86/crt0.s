section .text

extern main
extern exit
extern _init

global _start:function (_start.end - _start)
_start:
	call _init
	call main
	push eax
	call exit
.end: