section .text

extern main
extern exit
extern _init
extern _deinit

global _start:function (_start.end - _start)
_start:
	call _init
	call main
	push eax
	call _deinit
	call exit
.end: