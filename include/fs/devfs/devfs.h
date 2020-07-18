#ifndef __oneOS__FS__DEVFS__DEVFS_H
#define __oneOS__FS__DEVFS__DEVFS_H

#include <types.h>
#include <fs/vfs.h>


#define DEVFS_INODE_LEN (sizeof(struct devfs_inode))
struct devfs_inode {
    mode_t   mode;
    uint16_t uid;
    uint32_t size;
    uint32_t atime;
    uint32_t ctime;
    uint32_t mtime;
    uint32_t dtime;
    uint16_t gid;
    uint16_t links_count;
    uint32_t blocks;
    uint32_t flags;
    uint32_t osd1;

    /* NOTE: Instead of blocks here, we store devfs required things */
    uint32_t index;
    char* name;
    struct file_ops handlers;
    struct devfs_inode* parent;
    struct devfs_inode* prev;
    struct devfs_inode* next;
    struct devfs_inode* first;
    struct devfs_inode* last;
    uint32_t align_not_used;
    /* Block hack ends here */

    uint32_t generation;
    uint32_t file_acl;
    uint32_t dir_acl;
    uint32_t faddr;
    uint32_t osd2[3];
};
typedef struct devfs_inode devfs_inode_t;


void devfs_install();

devfs_inode_t* devfs_mkdir(dentry_t* dir, const char* name, uint32_t len);
devfs_inode_t* devfs_register(dentry_t* dir, const char* name, uint32_t len, mode_t mode, const struct file_ops* handlers);


#endif /* __oneOS__FS__DEVFS__DEVFS_H */