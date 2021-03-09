#ifndef _LIBC_SYS__TYPES__INTS_H
#define _LIBC_SYS__TYPES__INTS_H

#include <bits/types.h>

#ifndef __stdints_defined
#define __stdints_defined
typedef __int8_t int8_t;
typedef __int16_t int16_t;
typedef __int32_t int32_t;
typedef __int64_t int64_t;
typedef __uint8_t uint8_t;
typedef __uint16_t uint16_t;
typedef __uint32_t uint32_t;
typedef __uint64_t uint64_t;
#endif // __stdints_defined

#ifndef __dev_t_defined
#define __dev_t_defined
typedef __dev_t dev_t;
#endif // __dev_t_defined

#ifndef __uid_t_defined
#define __uid_t_defined
typedef __uid_t uid_t;
#endif // __uid_t_defined

#ifndef __gid_t_defined
#define __gid_t_defined
typedef __gid_t gid_t;
#endif // __gid_t_defined

#ifndef __ino_t_defined
#define __ino_t_defined
typedef __ino_t ino_t;
#endif // __ino_t_defined

#ifndef __ino64_t_defined
#define __ino64_t_defined
typedef __ino64_t ino64_t;
#endif // __ino64_t_defined

#ifndef __mode_t_defined
#define __mode_t_defined
typedef __mode_t mode_t;
#endif // __mode_t_defined

#ifndef __nlink_t_defined
#define __nlink_t_defined
typedef __nlink_t nlink_t;
#endif // __nlink_t_defined

#ifndef __off_t_defined
#define __off_t_defined
typedef __off_t off_t;
#endif // __off_t_defined

#ifndef __off64_t_defined
#define __off64_t_defined
typedef __off64_t off64_t;
#endif // __off64_t_defined

#ifndef __pid_t_defined
#define __pid_t_defined
typedef __pid_t pid_t;
#endif // __pid_t_defined

#ifndef __fsid_t_defined
#define __fsid_t_defined
typedef __fsid_t fsid_t;
#endif // __fsid_t_defined

#ifndef __time_t_defined
#define __time_t_defined
typedef __time_t time_t;
#endif // __time_t_defined
#endif // _LIBC_SYS__TYPES__INTS_H