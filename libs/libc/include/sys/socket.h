#ifndef _LIBC_SYS_SOCKET_H
#define _LIBC_SYS_SOCKET_H

#include <bits/sys/socket.h>
#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

int socket(int domain, int type, int protocol);
int bind(int sockfd, const char* name, int len);
int connect(int sockfd, const char* name, int len);

__END_DECLS

#endif // _LIBC_SYS_SOCKET_H