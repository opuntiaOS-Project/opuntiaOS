#ifndef __oneOS__LibC__UNISTD_H
#define __oneOS__LibC__UNISTD_H

#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

/* tasking */
int fork();
int execve(const char* path, char** argv, char** env);
int wait(int pid);

/* fs */
int read(int fd, char* buf, size_t count);
int write(int fd, const void* buf, size_t count);

__END_DECLS

#endif // __oneOS__LibC__UNISTD_H
