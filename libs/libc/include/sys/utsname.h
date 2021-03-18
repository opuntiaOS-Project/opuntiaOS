#ifndef _LIBC_SYS_UTSNAME_H
#define _LIBC_SYS_UTSNAME_H

#include <bits/sys/utsname.h>
#include <sys/cdefs.h>

__BEGIN_DECLS

int uname(utsname_t* buf);

__END_DECLS

#endif // _LIBC_SYS_UTSNAME_H