#include <cmd/system_commands.h>
#include <fs/vfs.h>
#include <mem/kmalloc.h>
#include <x86/tss.h>
#include <tasking/tasking.h>

void _syscmd_init_vfs();

// imitation of mkdir app
void _syscmd_mkdir(int argc, char *argv[]) {
    dentry_t* dir;
    if (vfs_resolve_path("/", &dir) < 0) {
        return;
    }

    int name_len = strlen(argv[1]);
    uint16_t dir_mode = EXT2_S_IFDIR | EXT2_S_IRUSR | EXT2_S_IWUSR | EXT2_S_IXUSR | EXT2_S_IRGRP | EXT2_S_IXGRP | EXT2_S_IROTH | EXT2_S_IXOTH;
    vfs_mkdir(dir, argv[1], name_len, dir_mode);
}

void _syscmd_ls(int argc, char *argv[]) {
    file_descriptor_t fd;
    dentry_t* dir;
    if (vfs_resolve_path(argv[1], &dir) < 0) {
        return;
    }

    if (vfs_open(dir, &fd) < 0) {
        dentry_put(dir);
        return;
    }
    
    dentry_put(dir);

    dirent_t tmp;
    while (vfs_getdirent(&fd, &tmp) == 0) {
        kprintf(tmp.name); kprintf("\n");
    }
    vfs_close(&fd);
}

void _syscmd_shutdown(int argc, char *argv[]) {
    clean_screen();
    kprintf("Shutting Down\n");
    eject_all_devices();
    clean_screen();
    kprintf("Off\n");
    while(1) {}
}

void umode(int argc, char *argv[]) {
    // set_proc2();
    tasking_start_init_proc();
}

// void open(int argc, char *argv[]) {
//     // set_proc2();
//     file_descriptor_t fd;
//     vfs_open((file_descriptor_t*)(0), argv[1], &fd);
//     kprintf("Inode: "); kprintd(fd.inode_index); kprintf("\n");
//     kprintf("Size: "); kprintd(fd.size); kprintf("\n");
    
//     char data[127];
//     memset(data, 0, sizeof(data));
//     vfs_read(&fd, (uint8_t*)&data, 1, 10);
//     kprintf(data);
// }

// void write(int argc, char *argv[]) {
//     // set_proc2();
//     file_descriptor_t fd;
//     vfs_open((file_descriptor_t*)(0), argv[1], &fd);
//     kprintf("Inode: "); kprintd(fd.inode_index); kprintf("\n");
//     kprintf("Size: "); kprintd(fd.size); kprintf("\n");
    
//     char data[2048];
//     for (int i = 0; i < 2048; i++) {
//         data[i] = '1';
//     } 
//     vfs_write(&fd, (uint8_t*)&data, 0, 2048);
//     // kprintf(data);

//     fd;
//     vfs_open((file_descriptor_t*)(0), argv[1], &fd);
//     kprintf("Inode: "); kprintd(fd.inode_index); kprintf("\n");
//     kprintf("Size: "); kprintd(fd.size); kprintf("\n");
// }

void read(int argc, char *argv[]) {
    // set_proc2();
    file_descriptor_t fd;
    dentry_t* file;
    if (vfs_resolve_path(argv[1], &file) < 0) {
        return;
    }
    if (vfs_open(file, &fd) < 0) {
        dentry_put(file);
        return;
    }
    dentry_put(file);
    
    kprintf("Inode: "); kprintd(fd.dentry->inode_indx); kprintf("\n");
    kprintf("Size: "); kprintd(fd.dentry->inode->size); kprintf("\n");
    
    char data[127];
    memset(data, 0, sizeof(data));
    vfs_read(&fd, (uint8_t*)&data, 0, fd.dentry->inode->size);
    kprintf(data);
    vfs_close(&fd);
}

void procfs(int argc, char *argv[]) {
    dentry_t* mp;
    if (vfs_resolve_path("/proc", &mp) < 0) {
        return;
    }
    vfs_mount(mp, new_virtual_device(DEVICE_STORAGE), 1);
    dentry_put(mp);
}

void umount(int argc, char *argv[]) {
    dentry_t* mp;
    if (vfs_resolve_path("/proc", &mp) < 0) {
        return;
    }
    vfs_umount(mp);
    dentry_put(mp);
}

void touch(int argc, char *argv[]) {
    dentry_t* dir;
    if (vfs_resolve_path(argv[1], &dir) < 0) {
        return;
    }

    int len = strlen(argv[2]);
    mode_t mode = EXT2_S_IFREG | EXT2_S_IRUSR | EXT2_S_IWUSR | EXT2_S_IXUSR | EXT2_S_IRGRP | EXT2_S_IXGRP | EXT2_S_IROTH | EXT2_S_IXOTH;

    if (vfs_create(dir, argv[2], len, mode) < 0) {
        return;
    }
}

void write(int argc, char *argv[]) {
    dentry_t* file;
    file_descriptor_t fd;
    if (vfs_resolve_path(argv[1], &file) < 0) {
        return;
    }

    if (vfs_open(file, &fd) < 0) {
        return;
    }

    int len = strlen(argv[2]);
    if (vfs_write(&fd, argv[2], 0, len) < 0) {
        return;
    }

    vfs_close(&fd);
}

void rm(int argc, char *argv[]) {
    dentry_t* file;
    file_descriptor_t fd;
    if (vfs_resolve_path(argv[1], &file) < 0) {
        return;
    }
    dentry_put(file);
    if (vfs_rm(file) < 0) {
        kprintf("Doesn't del");
        return;
    }
}

void dentries_stat(int argc, char *argv[]) {
    kprintf("Cached dentried: %d\n", dentry_stat_cached_count());
}

void syscmd_init() {
    cmd_register("mkdir", _syscmd_mkdir);
    cmd_register("ls", _syscmd_ls);
    cmd_register("shutdown", _syscmd_shutdown);
    cmd_register("umode", umode);

    cmd_register("cat", read);
    cmd_register("procfs", procfs);
    cmd_register("umount", umount);

    cmd_register("touch", touch);
    cmd_register("rm", rm);

    cmd_register("write", write);
    cmd_register("dentries", dentries_stat);
}
