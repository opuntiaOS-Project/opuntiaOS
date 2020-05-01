#include <cmd/system_commands.h>
#include <fs/vfs.h>
#include <mem/malloc.h>
#include <x86/tss.h>
#include <tasking/tasking.h>

void _syscmd_init_vfs();

// imitation of mkdir app
void _syscmd_mkdir(int argc, char *argv[]) {
    file_descriptor_t fd;
    vfs_open((file_descriptor_t*)(0), "/", &fd);
    vfs_mkdir(&fd, argv[1]);
}

void _syscmd_shutdown(int argc, char *argv[]) {
    clean_screen();
    printf("Shutting Down\n");
    eject_all_devices();
    clean_screen();
    printf("Off\n");
    while(1) {}
}

void umode(int argc, char *argv[]) {
    // set_proc2();
    tasking_start_init_proc();
}

void open(int argc, char *argv[]) {
    // set_proc2();
    file_descriptor_t fd;
    vfs_open((file_descriptor_t*)(0), argv[1], &fd);
    printf("Inode: "); printd(fd.inode_index); printf("\n");
    printf("Size: "); printd(fd.size); printf("\n");
    
    char data[127];
    memset(data, 0, sizeof(data));
    vfs_read(&fd, (uint8_t*)&data, 1, 10);
    printf(data);
}

void write(int argc, char *argv[]) {
    // set_proc2();
    file_descriptor_t fd;
    vfs_open((file_descriptor_t*)(0), argv[1], &fd);
    printf("Inode: "); printd(fd.inode_index); printf("\n");
    printf("Size: "); printd(fd.size); printf("\n");
    
    char data[2048];
    for (int i = 0; i < 2048; i++) {
        data[i] = '1';
    } 
    vfs_write(&fd, (uint8_t*)&data, 0, 2048);
    // printf(data);

    fd;
    vfs_open((file_descriptor_t*)(0), argv[1], &fd);
    printf("Inode: "); printd(fd.inode_index); printf("\n");
    printf("Size: "); printd(fd.size); printf("\n");
}

void syscmd_init() {
    cmd_register("mkdir", _syscmd_mkdir);
    cmd_register("shutdown", _syscmd_shutdown);
    cmd_register("umode", umode);

    cmd_register("open", open);
    cmd_register("wr", write);
}
