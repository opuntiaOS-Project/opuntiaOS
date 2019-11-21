#include <cmd/system_commands.h>
#include <fs/vfs.h>

void _syscmd_init_vfs();

// imitation of LS app
void _syscmd_ls(int argc, char *argv[]) {
    vfs_lookup_dir("/");
}

// imitation of mkdir app
void _syscmd_mkdir(int argc, char *argv[]) {
    printf(argv[1]);
    vfs_create_dir("/", argv[1]);
}

void syscmd_init() {
    cmd_register("ls", _syscmd_ls);
    cmd_register("mkdir", _syscmd_mkdir);
}
