#ifndef __oneOS__LibC__STDLIB_H
#define __oneOS__LibC__STDLIB_H

#include <sys/types.h>
#include <syscalls.h>

int posix_openpt(int flags);
int ptsname_r(int fd, char* buf, size_t buflen);
char* ptsname(int fd);

#endif // __oneOS__LibC__STDLIB_H
