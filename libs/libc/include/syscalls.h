#ifndef _LIBC_SYSCALLS_H
#define _LIBC_SYSCALLS_H

#include <bits/fcntl.h>
#include <bits/ioctls.h>
#include <bits/select.h>
#include <bits/socket.h>
#include <bits/sys/mman.h>
#include <bits/sys/stat.h>
#include <bits/sys/utsname.h>
#include <bits/syscalls.h>
#include <bits/thread.h>
#include <bits/time.h>
#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

int syscall(sysid_t sysid, int p1, int p2, int p3, int p4, int p5);

int close(int fd);

int lseek(int fd, uint32_t off, int whence);
pid_t getpid();
int kill(pid_t pid, int signo);

int raise(int signo);
int sigaction(int signo, void* callback);
int socket(int domain, int type, int protocol);
int bind(int sockfd, const char* name, int len);
int connect(int sockfd, const char* name, int len);
int getdents(int fd, char* buf, int len);
int ioctl(int fd, uint32_t cmd, uint32_t arg);
int setpgid(pid_t cmd, pid_t arg);
pid_t getpgid(pid_t arg);
int select(int nfds, fd_set_t* readfds, fd_set_t* writefds, fd_set_t* exceptfds, timeval_t* timeout);
int fstat(int nfds, fstat_t* stat);

int system_pthread_create(thread_create_params_t* params);

int shared_buffer_create(uint8_t** buffer, size_t size);
int shared_buffer_get(int id, uint8_t** buffer);
int shared_buffer_free(int id);

__END_DECLS

#endif /* _LIBC_SYSCALLS_H */