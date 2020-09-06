#ifndef __oneOS__libsystem__SYSCALLS_H
#define __oneOS__libsystem__SYSCALLS_H

#include "types.h"
#include "../../include/syscall_structs.h"

#define O_RDONLY 0x1
#define O_WRONLY 0x2
#define O_RDWR 0x4
#define O_DIRECTORY 0x8

extern int errno;

int open(const char *pathname, int flags);
int read(int fd, char *buf, size_t count);
int write(int fd, const void *buf, size_t count);
int fork();
int wait(int pid);
int execve(char* path, char** argv, char** env);

int raise(int signo);
int sigaction(int signo, void* callback);
int mmap(mmap_params_t* params);
int socket(int domain, int type, int protocol);
int bind(int sockfd, char* name, int len);
int connect(int sockfd, char* name, int len);
int getdents(int fd, char* buf, int len);
int system_pthread_create(thread_create_params_t* params);

#endif /* __oneOS__libsystem__SYSCALLS_H */