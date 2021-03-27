#include <sys/socket.h>
#include <sysdep.h>

int socket(int domain, int type, int protocol)
{
    return DO_SYSCALL_3(SYSSOCKET, domain, type, protocol);
}

int bind(int sockfd, const char* name, int len)
{
    return DO_SYSCALL_3(SYSBIND, sockfd, name, len);
}

int connect(int sockfd, const char* name, int len)
{
    return DO_SYSCALL_3(SYSCONNECT, sockfd, name, len);
}