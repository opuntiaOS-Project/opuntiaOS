#ifndef _LIBC_BITS_TYPES_H
#define _LIBC_BITS_TYPES_H

typedef char __int8_t;
typedef short __int16_t;
typedef int __int32_t;
typedef long long __int64_t;
typedef unsigned char __uint8_t;
typedef unsigned short __uint16_t;
typedef unsigned int __uint32_t;
typedef unsigned long long __uint64_t;

typedef __uint32_t __dev_t; /* Type of device numbers.  */
typedef __uint32_t __uid_t; /* Type of user identifications.  */
typedef __uint32_t __gid_t; /* Type of group identifications.  */
typedef __uint32_t __ino_t; /* Type of file serial numbers.  */
typedef __uint64_t __ino64_t; /* Type of file serial numbers (LFS).*/
typedef __uint16_t __mode_t; /* Type of file attribute bitmasks.  */
typedef __uint32_t __nlink_t; /* Type of file link counts.  */
typedef __int64_t __off64_t; /* Type of file sizes and offsets (LFS).  */
typedef __uint32_t __pid_t; /* Type of process identifications.  */
typedef __uint32_t __fsid_t; /* Type of file system IDs.  */
typedef __uint32_t __time_t; /* Seconds since the Epoch.  */

#if defined(__x86_64__) || defined(__aarch64__) || (defined(__riscv) && (__riscv_xlen == 64))
#define BITS64
typedef __int64_t __off_t; /* Type of file sizes and offsets.  */

#ifndef LONG_TYPE_SIZE
#define LONG_TYPE_SIZE (8)
#endif

#else
#define BITS32
typedef __int32_t __off_t; /* Type of file sizes and offsets.  */

#ifndef LONG_TYPE_SIZE
#define LONG_TYPE_SIZE (4)
#endif

#endif

#endif // _LIBC_BITS_TYPES_H