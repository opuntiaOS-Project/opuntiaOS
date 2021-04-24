#ifndef _LIBC_CTYPE_H
#define _LIBC_CTYPE_H

#include <stddef.h>
#include <sys/_structs.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

#define _U 01
#define _L 02
#define _N 04
#define _S 010
#define _P 020
#define _C 040
#define _X 0100
#define _B 0200

extern const char __ctypes[256];

int tolower(int);
int toupper(int);
int isalnum(int);
int isalpha(int);
int iscntrl(int);
int isdigit(int);
int isxdigit(int);
int isspace(int);
int ispunct(int);
int isprint(int);
int isgraph(int);
int islower(int);
int isupper(int);

#define isalnum(c) (__ctypes[(unsigned char)(c)] & (_U | _L | _N))
#define isalpha(c) (__ctypes[(unsigned char)(c)] & (_U | _L))
#define iscntrl(c) (__ctypes[(unsigned char)(c)] & (_C))
#define isdigit(c) (__ctypes[(unsigned char)(c)] & (_N))
#define isxdigit(c) (__ctypes[(unsigned char)(c)] & (_N | _X))
#define isspace(c) (__ctypes[(unsigned char)(c)] & (_S))
#define ispunct(c) (__ctypes[(unsigned char)(c)] & (_P))
#define isprint(c) (__ctypes[(unsigned char)(c)] & (_P | _U | _L | _N | _B))
#define isgraph(c) (__ctypes[(unsigned char)(c)] & (_P | _U | _L | _N))
#define islower(c) ((__ctypes[(unsigned char)(c)] & (_U | _L)) == _L)
#define isupper(c) ((__ctypes[(unsigned char)(c)] & (_U | _L)) == _U)

__END_DECLS

#endif /* _LIBC_CTYPE_H */