#ifndef __oneOS__FS__DEVFS__DEVFS_H
#define __oneOS__FS__DEVFS__DEVFS_H

#include <types.h>
#include <fs/vfs.h>

typedef struct {
    mode_t   mode;
    uint16_t uid;
    uint32_t size;
    uint16_t gid;
    uint16_t links_count;
    uint32_t flags;
} devfs_inode_t;

struct devfs_entry {
    devfs_inode_t* inode;
    struct devfs_entry* parent;
    struct devfs_entry* prev;
    struct devfs_entry* next;
    struct devfs_entry* first;
    struct devfs_entry* last;
};
typedef struct devfs_entry devfs_entry_t;

void devfs_install();

devfs_entry_t* devfs_mkdir(devfs_entry_t* dir, const char* name, uint32_t len);
devfs_entry_t* devfs_register(devfs_entry_t* dir, const char* name, uint32_t len, mode_t mode, file_ops_t* handlers);

#endif /* __oneOS__FS__DEVFS__DEVFS_H */