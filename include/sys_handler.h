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
void sys_write(trapframe_t *tf);
void sys_open(trapframe_t* tf);
void sys_close(trapframe_t *tf);
void sys_waitpid(trapframe_t *tf);
void sys_exec(trapframe_t *tf);
void sys_sigaction(trapframe_t* tf);
void sys_sigreturn(trapframe_t* tf);
void sys_raise(trapframe_t* tf);
void sys_getpid(trapframe_t* tf);
void sys_mmap(trapframe_t* tf);
void sys_munmap(trapframe_t* tf);
void sys_socket(trapframe_t* tf);
void sys_bind(trapframe_t* tf);
void sys_connect(trapframe_t* tf);
void sys_getdents(trapframe_t* tf);
void sys_ioctl(trapframe_t* tf);
void sys_setpgid(trapframe_t* tf);
void sys_getpgid(trapframe_t* tf);
void sys_create_thread(trapframe_t* tf);

void sys_none(trapframe_t* tf);

#endif // __oneOS__INTERRUPTS__SYS_HANDLER_H
