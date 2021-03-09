#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <syscalls.h>
#include <unistd.h>

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
    struct linux_dirent* d;
    int bpos;
    char d_type;
    char has_path = 0;
    char show_inodes = 0;
    char show_private = 0;

    for (int i = 1; i < argc; i++) {
        if (memcmp(argv[i], "-i", 3) == 0) {
            show_inodes = 1;
        } else if (memcmp(argv[i], "-a", 3) == 0) {
            show_private = 1;
        } else {
            has_path = 1;
        }
    }

    if (has_path) {
        fd = open(argv[1], O_RDONLY | O_DIRECTORY);
    } else {
        fd = open(".", O_RDONLY | O_DIRECTORY);
    }

    if (fd < 0) {
        printf("ls: can't open file\n");
        return -1;
    }
    for (;;) {
        nread = getdents(fd, buf, BUF_SIZE);
        if (nread < 0) {
            printf("ls: can't read dir\n");
            return -1;
        }

        if (nread == 0)
            break;

        for (bpos = 0; bpos < nread;) {
            d = (struct linux_dirent*)(buf + bpos);
            if (((char*)&d->name)[0] != '.' || show_private) {
                printf("%s", &d->name);
                if (show_inodes) {
                    printf(" %d", d->inode);
                }
                printf("\n");
            }
            bpos += d->rec_len;
        }
    }
    return 0;
}