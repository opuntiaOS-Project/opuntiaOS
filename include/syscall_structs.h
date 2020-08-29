#ifndef __oneOS__API_H
#define __oneOS__API_H

#ifndef __oneOS__TYPES_H
#define __oneOS__TYPES_H

typedef unsigned long  uint64_t;
typedef unsigned int   uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char  uint8_t;
typedef long           int64_t;
typedef int            int32_t;
typedef short          int16_t;
typedef char           int8_t;
typedef char           bool;

#define true  1
#define false 0

#endif

enum __sysid {
    SYSPRINT = 0,
    SYSEXTT,
    SYSFORK,
    SYSREAD,
    SYSWRITE,
    SYSOPEN,
    SYSCLOSE,
    SYSWAITPID,
    SYSCREAT,
    SYSLINK,
    SYSUNLINK,
    SYSEXEC,
    SYSSIGACTION,
    SYSSIGRETURN,
    SYSRAISE,
    SYSMMAP,
    SYSMUNMAP,
    SYSSOCKET,
    SYSBIND,
    SYSCONNECT,
};
typedef enum __sysid sysid_t;

#define MAP_SHARED 0x01
#define MAP_PRIVATE 0x02
#define MAP_FIXED 0x10
#define MAP_ANONYMOUS 0x20
#define MAP_STACK 0x40

#define PROT_READ 0x1
#define PROT_WRITE 0x2
#define PROT_EXEC 0x4
#define PROT_NONE 0x0

struct mmap_params {
    uint32_t addr;
    uint32_t size;
    uint32_t alignment;
    uint32_t prot;
    uint32_t flags;
    uint32_t fd;
    uint32_t offset;
};
typedef struct mmap_params mmap_params_t;

/**
 * SOCKETS
 */

enum SOCK_DOMAINS {
    PF_LOCAL,
    PF_INET,
    PF_INET6,
    PF_IPX,
    PF_NETLINK,
    PF_X25,
    PF_AX25,
    PF_ATMPVC,
    PF_APPLETALK,
    PF_PACKET,
};

enum SOCK_TYPES {
    SOCK_STREAM,
    SOCK_DGRAM,
    SOCK_SEQPACKET,
    SOCK_RAW,
    SOCK_RDM,
    SOCK_PACKET,
};


#endif /* __oneOS__API_H */