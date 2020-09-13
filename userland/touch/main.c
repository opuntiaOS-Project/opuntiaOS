#include <libsystem/syscalls.h>

int main(int argc, char** argv)
{
    int fd;

    if (argc < 2) {
        write(1, "Usage: touch files...\n", 22);
        return 0;
    }

    mode_t std_mode = EXT2_S_IFREG | EXT2_S_IRUSR | EXT2_S_IWUSR | EXT2_S_IXUSR | EXT2_S_IRGRP | EXT2_S_IXGRP | EXT2_S_IROTH | EXT2_S_IXOTH;

    for (int i = 1; i < argc; i++) {
        if ((fd = creat(argv[i], std_mode)) < 0) {
            write(1, "touch: failed to create\n", 24);
            break;
        }
        close(fd);
    }

    return 0;
}