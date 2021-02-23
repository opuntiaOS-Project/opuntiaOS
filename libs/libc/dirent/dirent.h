#ifndef __oneOS__LibC__DIRENT_H
#define __oneOS__LibC__DIRENT_H

#include <sys/_structs.h>
#include <sys/types.h>

struct __dirstream {
    int fd;
    size_t size; /* size of dir data */
    size_t allocated; /* size of data holder block */
    size_t offset;
};
typedef struct __dirstream DIR;

struct __dirent {
    uint32_t d_ino; /* Inode number */
    uint32_t d_off; /* Offset to next linux_dirent */
    uint16_t d_reclen; /* Length of this linux_dirent */
    char d_name[]; /* Filename (null-terminated) */
};
typedef struct __dirent dirent_t;

#endif /* __oneOS__LibC__DIRENT_H */