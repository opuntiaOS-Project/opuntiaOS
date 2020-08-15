#ifndef __oneOS__libsystem__SYSCALLS_H
#define __oneOS__libsystem__SYSCALLS_H

#include "types.h"

void print(int value);

enum OPEN_MODE {
    O_RDONLY,
    O_WRONLY,
    O_RDWR
};
int open(const char *pathname, int flags);
int read(int fd, char *buf, size_t count);
int write(int fd, const void *buf, size_t count);
int fork();
int wait(int pid);
int exec(char* path);

int raise(int signo);
int sigaction(int signo, void* callback);

#endif /* __oneOS__libsystem__SYSCALLS_H */