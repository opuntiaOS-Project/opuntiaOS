#ifndef __oneOS__INTERRUPTS__SYS_HANDLER_H
#define __oneOS__INTERRUPTS__SYS_HANDLER_H

#include <types.h>
#include <x86/idt.h>
#include <drivers/display.h>

#define SYSCALL 0x80

void sys_handler(trapframe_t *tf);
void sys_restart_syscall(trapframe_t *tf);
void sys_exit(trapframe_t *tf);
void sys_fork(trapframe_t *tf);
void sys_read(trapframe_t *tf);
int sys_open(trapframe_t* tf);
int sys_close(trapframe_t *tf);
void sys_exec(trapframe_t *tf);
void sys_sigaction(trapframe_t* tf);
void sys_sigreturn(trapframe_t* tf);
void sys_raise(trapframe_t* tf);

#endif // __oneOS__INTERRUPTS__SYS_HANDLER_H
