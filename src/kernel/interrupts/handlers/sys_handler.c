#include <sys_handler.h>
#include <tasking/tasking.h>

// 32 bit Linux's syscalls

void sys_handler(trapframe_t *tf) {
    const void *syscalls[] = { 
        sys_restart_syscall,
        sys_exit,
        sys_fork,
        sys_read,
        sys_open,
        sys_close,
        sys_exec,
    };
    void (*callee)(trapframe_t*) = (void*)syscalls[tf->eax];
    callee(tf);
}
void sys_restart_syscall(trapframe_t *tf) {
    kprintd(tf->ebx);
}
void sys_exit(trapframe_t *tf) {
    
}
void sys_fork(trapframe_t *tf) {
    tasking_fork();
}
void sys_read(trapframe_t *tf) {
    
}
void sys_open(trapframe_t *tf) {
    
}
void sys_close(trapframe_t *tf) {

}
void sys_exec(trapframe_t *tf) {
    tasking_exec();
}

