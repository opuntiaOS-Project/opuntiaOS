#ifndef _KERNEL_LIBKERN_BITS_SYS_UTSNAME_H
#define _KERNEL_LIBKERN_BITS_SYS_UTSNAME_H

#define UTSNAME_ENTRY_LEN 65

struct utsname {
    char sysname[UTSNAME_ENTRY_LEN];
    char nodename[UTSNAME_ENTRY_LEN];
    char release[UTSNAME_ENTRY_LEN];
    char version[UTSNAME_ENTRY_LEN];
    char machine[UTSNAME_ENTRY_LEN];
};
typedef struct utsname utsname_t;

#endif // _KERNEL_LIBKERN_BITS_SYS_UTSNAME_H