#include <syscalls.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    int fd;

    if (argc < 2) {
        write(1, "Usage: touch files...\n", 22);
        return 0;
    }

    mode_t std_mode = S_IFREG | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;

    for (int i = 1; i < argc; i++) {
        if ((fd = creat(argv[i], std_mode)) < 0) {
            write(1, "touch: failed to create\n", 24);
            break;
        }
        close(fd);
    }

    return 0;
}