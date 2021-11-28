#include <sys/ptrace.h>
#include <sysdep.h>

int ptrace(ptrace_request_t request, pid_t pid, void* addr, void* data)
{
    int res = DO_SYSCALL_4(SYS_PTRACE, request, pid, addr, data);
    RETURN_WITH_ERRNO(res, 0, res);
}