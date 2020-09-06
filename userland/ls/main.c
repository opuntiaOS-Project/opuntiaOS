#include <libsystem/syscalls.h>

#define BUF_SIZE 1024

struct linux_dirent {
    uint32_t inode;
    uint16_t rec_len;
    uint8_t name_len;
    uint8_t file_type;
    char* name;
};

int main(int argc, char** argv)
{
    int fd, nread;
    char buf[BUF_SIZE];
    struct linux_dirent *d;
    int bpos;
    char d_type;

    if (argc > 1) {
        fd = open(argv[1], O_RDONLY | O_DIRECTORY);
    } else {
        write(1, "No path\n", 8);
        return -1;
    }
    if (fd < 0) {
        write(1, "-fd\n", 4);
        return -1;
    }
    for(;;) {
        nread = getdents(fd, buf, BUF_SIZE);
        if (nread < 0) {
            write(1, "Err\n", 4);
            return -1;
        }

        if (nread == 0)
            break;

        for (bpos = 0; bpos < nread;) {
            d = (struct linux_dirent *) (buf + bpos);
            write(1, &d->name, d->name_len + 1);
            write(1, "\n", 1);
            // d_type = *(buf + bpos + d->d_reclen - 1);
            bpos += d->rec_len;
        }
    }
    return 0;
}