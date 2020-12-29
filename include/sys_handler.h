#ifndef __oneOS__INTERRUPTS__SYS_HANDLER_H
#define __oneOS__INTERRUPTS__SYS_HANDLER_H

#include <drivers/display.h>
#include <syscall_structs.h>
#include <types.h>
#include <x86/idt.h>

#define SYSCALL 0x80

#define ksys1(sys_id, a) ksyscall_impl(sys_id, a, 0, 0, 0);
#define ksys2(sys_id, a, b) ksyscall_impl(sys_id, a, b, 0, 0);
#define ksys3(sys_id, a, b, c) ksyscall_impl(sys_id, a, b, c, 0);
#define ksys4(sys_id, a, b, c, d) ksyscall_impl(sys_id, a, b, c, d);
int ksyscall_impl(int sys_id, int a, int b, int c, int d);

void sys_handler(trapframe_t* tf);
void sys_restart_syscall(trapframe_t* tf);
void sys_exit(trapframe_t* tf);
void sys_fork(trapframe_t* tf);
void sys_read(trapframe_t* tf);
void sys_write(trapframe_t* tf);
void sys_open(trapframe_t* tf);
void sys_close(trapframe_t* tf);
void sys_waitpid(trapframe_t* tf);
void sys_creat(trapframe_t* tf);
void sys_exec(trapframe_t* tf);
void sys_chdir(trapframe_t* tf);
void sys_sigaction(trapframe_t* tf);
void sys_sigreturn(trapframe_t* tf);
void sys_raise(trapframe_t* tf);
void sys_lseek(trapframe_t* tf);
void sys_getpid(trapframe_t* tf);
void sys_kill(trapframe_t* tf);
void sys_mkdir(trapframe_t* tf);
void sys_rmdir(trapframe_t* tf);
void sys_unlink(trapframe_t* tf);
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
void sys_sleep(trapframe_t* tf);
void sys_select(trapframe_t* tf);
void sys_fstat(trapframe_t* tf);
void sys_sched_yield(trapframe_t* tf);
void sys_shbuf_create(trapframe_t* tf);
void sys_shbuf_get(trapframe_t* tf);
void sys_shbuf_free(trapframe_t* tf);

void sys_none(trapframe_t* tf);

#endif // __oneOS__INTERRUPTS__SYS_HANDLER_H
