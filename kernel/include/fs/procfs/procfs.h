#ifndef _KERNEL_FS_PROCFS_PROCFS_H
#define _KERNEL_FS_PROCFS_PROCFS_H

#include <fs/vfs.h>
#include <libkern/types.h>

struct procfs_files {
    char* name;
    mode_t mode;
    const file_ops_t* ops;
};
typedef struct procfs_files procfs_files_t;

#define DEVFS_INODE_LEN (sizeof(struct devfs_inode))
struct procfs_inode {
    mode_t mode;
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

    /* NOTE: Instead of blocks here, we store procfs required things */
    uint32_t index;
    const struct file_ops* ops;
    char* name;
    struct file_ops handlers;
    struct devfs_inode* parent;
    struct devfs_inode* prev;
    struct devfs_inode* next;
    struct devfs_inode* first;
    struct devfs_inode* last;
    /* Block hack ends here */

    uint32_t generation;
    uint32_t file_acl;
    uint32_t dir_acl;
    uint32_t faddr;
    uint32_t osd2[3];
};
typedef struct procfs_inode procfs_inode_t;

void procfs_install();
int procfs_mount();

uint32_t procfs_root_get_pid_from_inode_index(uint32_t inode_index);

static inline uint32_t procfs_get_inode_index(const uint32_t level, uint32_t main)
{
    return (level << 28) | (main & 0x0fffffff);
}

#endif // _KERNEL_FS_PROCFS_PROCFS_H