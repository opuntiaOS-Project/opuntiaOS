#ifndef __oneOS__FS__EXT2__EXT2_H
#define __oneOS__FS__EXT2__EXT2_H

#include <fs/vfs.h>
#include <types.h>

#define SUPERBLOCK_START 1024
#define SUPERBLOCK_LEN (sizeof(superblock_t))
typedef struct {
    uint32_t inodes_count;
    uint32_t blocks_count;
    uint32_t r_blocks_count;
    uint32_t free_blocks_count;
    uint32_t free_inodes_count;
    uint32_t first_data_block;
    uint32_t log_block_size;
    uint32_t log_frag_size;
    uint32_t blocks_per_group;
    uint32_t frags_per_group;
    uint32_t inodes_per_group;
    uint32_t mtime;
    uint32_t wtime;
    uint16_t mnt_count;
    uint16_t max_mnt_count;
    uint16_t magic;
    uint16_t state;
    uint16_t errors;
    uint16_t minor_rev_level;
    uint32_t lastcheck;
    uint32_t checkinterval;
    uint32_t creator_os;
    uint32_t rev_level;
    uint16_t def_resuid;
    uint16_t def_resgid;

    uint32_t first_ino;
    uint16_t inode_size;
    uint16_t block_group_nr;
    uint32_t feature_compat;
    uint32_t feature_incompat;
    uint32_t feature_ro_compat;
    uint8_t  uuid[16];
    uint8_t  volume_name[16];
    uint8_t  last_mounted[64];
    uint32_t algo_bitmap;

    uint8_t  prealloc_blocks;
    uint8_t  prealloc_dir_blocks;

    // current jurnalling is unsupported
    uint8_t unused[1024-206];
} superblock_t;

#define GROUP_LEN (sizeof(group_desc_t))
typedef struct {
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;
    uint16_t free_blocks_count;
    uint16_t free_inodes_count;
    uint16_t used_dirs_count;
    uint16_t pad;
    uint8_t  reserved[12];
} group_desc_t;

#define EXT2_S_IFSOCK 0xC000
#define EXT2_S_IFLNK 0xA000
#define EXT2_S_IFREG 0x8000
#define EXT2_S_IFBLK 0x6000
#define EXT2_S_IFDIR 0x4000
#define EXT2_S_IFCHR 0x2000
#define EXT2_S_IFIFO 0x1000

#define EXT2_S_ISUID 0x0800
#define EXT2_S_ISGID 0x0400
#define EXT2_S_ISVTX 0x0200

#define EXT2_S_IRUSR 0x0100
#define EXT2_S_IWUSR 0x0080
#define EXT2_S_IXUSR 0x0040
#define EXT2_S_IRGRP 0x0020
#define EXT2_S_IWGRP 0x0010
#define EXT2_S_IXGRP 0x0008
#define EXT2_S_IROTH 0x0004
#define EXT2_S_IWOTH 0x0002
#define EXT2_S_IXOTH 0x0001

#define INODE_LEN (sizeof(inode_t))
#define INODES_RESERVED 11
typedef struct {
    uint16_t mode;
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
    uint32_t block[15];
    uint32_t generation;
    uint32_t file_acl;
    uint32_t dir_acl;
    uint32_t faddr;
    uint32_t osd2[3];
} inode_t;

#define DIR_ENTRY_LEN (sizeof(dir_entry_t))
typedef struct {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char* name; // TODO maybe not used?
} dir_entry_t;


void ext2_install();
bool ext2_recognize_drive(vfs_device_t *dev);
void ext2_stub();

int ext2_read_inode(vfs_device_t *dev, uint32_t inode_id, inode_t *inode);
int ext2_has_in_dir(vfs_device_t *dev, uint32_t block_idx, const char *path, uint32_t *found_inode_idx);
int ext2_scan_dir(vfs_device_t *dev, inode_t inode, const char *path, uint32_t *res_inode_idx);
int ext2_get_inode_idx(vfs_device_t *dev, const char *path, uint32_t *file_inode);
int ext2_open(vfs_device_t* dev, file_descriptor_t* base, const char* path, file_descriptor_t* fd);
int ext2_read(vfs_device_t *dev, file_descriptor_t *fd, uint8_t* buf, uint32_t start, uint32_t len);
int ext2_write(vfs_device_t *dev, file_descriptor_t *fd, uint8_t* buf, uint32_t start, uint32_t len);
int ext2_mkdir(vfs_device_t *dev, file_descriptor_t *fd, const char* name);

#endif // __oneOS__FS__EXT2__EXT2_H