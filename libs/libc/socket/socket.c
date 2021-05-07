#include <sys/socket.h>
#include <sysdep.h>

int socket(int domain, int type, int protocol)
{
    int res = DO_SYSCALL_3(SYS_SOCKET, domain, type, protocol);
    RETURN_WITH_ERRNO(res, res, -1);
}

int bind(int sockfd, const char* name, int len)
{
    int res = DO_SYSCALL_3(SYS_BIND, sockfd, name, len);
    RETURN_WITH_ERRNO(res, 0, -1);
}

int connect(int sockfd, const char* name, int len)
{
    int res = DO_SYSCALL_3(SYS_CONNECT, sockfd, name, len);
    RETURN_WITH_ERRNO(res, 0, -1);
}