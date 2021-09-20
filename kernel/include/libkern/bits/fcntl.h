#ifndef _KERNEL_LIBKERN_BITS_FCNTL_H
#define _KERNEL_LIBKERN_BITS_FCNTL_H

#define SEEK_SET 0x1
#define SEEK_CUR 0x2
#define SEEK_END 0x3

/* OPEN */
#define O_RDONLY 0x1
#define O_WRONLY 0x2
#define O_RDWR (O_RDONLY | O_WRONLY)
#define O_DIRECTORY 0x4
#define O_CREAT 0x8
#define O_TRUNC 0x10
#define O_APPEND 0x20
#define O_EXCL 0x40
#define O_EXEC 0x80

#endif // _KERNEL_LIBKERN_BITS_FCNTL_H