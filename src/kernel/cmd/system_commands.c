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
        return;
    }

    dirent_t tmp;
    while (vfs_getdirent(&fd, &tmp) == 0) {
        kprintf(tmp.name); kprintf("\n");
    }
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
    kprintf("endndn");
    vfs_open(file, &fd);
    kprintf("Inode: "); kprintd(fd.dentry->inode_indx); kprintf("\n");
    kprintf("Size: "); kprintd(fd.dentry->inode->size); kprintf("\n");
    
    char data[127];
    memset(data, 0, sizeof(data));
    vfs_read(&fd, (uint8_t*)&data, 1, 10);
    kprintf(data);
}

void syscmd_init() {
    cmd_register("mkdir", _syscmd_mkdir);
    cmd_register("ls", _syscmd_ls);
    cmd_register("shutdown", _syscmd_shutdown);
    cmd_register("umode", umode);

    cmd_register("cat", read);

    ;
    // cmd_register("wr", write);
}
