#include <cmd/system_commands.h>
#include <fs/vfs.h>
#include <mem/malloc.h>

void _syscmd_init_vfs();

// imitation of LS app
void _syscmd_ls(int argc, char *argv[]) {
    vfs_element_t vfs_buf[16]; // 16 tmp value (will be replaced with std value)
    char str_buf[VFS_MAX_FILENAME + VFS_MAX_FILENAME_EXT + 2];
    uint32_t n = vfs_lookup_dir("/", vfs_buf);
    for (int i = 0; i < n; i++) {
        uint32_t fname_len = strlen(vfs_buf[i].filename);
        uint32_t fext_len = strlen(vfs_buf[i].filename_ext);

        memcpy(str_buf, vfs_buf[i].filename, fname_len);

        if (vfs_buf[i].attributes >= 0x10) {
            str_buf[fname_len] = '/';
        } else {
            str_buf[fname_len] = '.';
            memcpy(str_buf+fname_len+1, vfs_buf[i].filename_ext, fext_len);
        }

        str_buf[fname_len + fext_len + 1] = '\0';
        printf(str_buf);
        printf("\n");
    }
}

// imitation of mkdir app
void _syscmd_mkdir(int argc, char *argv[]) {
    vfs_create_dir("/", argv[1]);
}

void _syscmd_echo(int argc, char *argv[]) {
    char *dd = vfs_read_file("/", argv[1], 0, -1);
    printf(dd);
    kfree(dd);
}

void _syscmd_write(int argc, char *argv[]) {
    vfs_write_file("/", argv[1], "hello", 5);
}

void syscmd_init() {
    cmd_register("ls", _syscmd_ls);
    cmd_register("mkdir", _syscmd_mkdir);
    cmd_register("cat", _syscmd_echo);
    cmd_register("echo", _syscmd_write);
}
