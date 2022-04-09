#ifndef _LIBC_SYS_STAT_H
#define _LIBC_SYS_STAT_H

#include <bits/sys/stat.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

int mkdir(const char* path);
int fstat(int nfds, stat_t* stat);

__END_DECLS

#endif