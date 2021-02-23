#ifndef __oneOS__LibC__STDLIB_H
#define __oneOS__LibC__STDLIB_H

#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

/* malloc */
extern void* malloc(size_t);
extern void free(void*);
extern void* calloc(size_t, size_t);
extern void* realloc(void*, size_t);

/* exit */
void exit(int status);

/* pts */
int posix_openpt(int flags);
int ptsname_r(int fd, char* buf, size_t buflen);
char* ptsname(int fd);

__END_DECLS

#endif // __oneOS__LibC__STDLIB_H
