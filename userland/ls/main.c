#include <string.h>
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

void print_int(int num)
{
    int id = 0;
    char buf[32];
    if (!num) {
        write(1, "0", 1);
    }
    while (num) {
        buf[id++] = (num % 10) + '0';
        num /= 10;
    }

    for (int i = 0; i < id / 2; i++) {
        char tmp = buf[i];
        buf[i] = buf[id - i - 1];
        buf[id - i - 1] = tmp;
    }

    buf[id] = '\0';
    write(1, buf, id);
}

int main(int argc, char** argv)
{
    int fd, nread;
    char buf[BUF_SIZE];
    struct linux_dirent* d;
    int bpos;
    char d_type;
    char has_path = false;
    char show_inodes = false;
    char show_private = false;

    for (int i = 1; i < argc; i++) {
        if (memcmp(argv[i], "-i", 3) == 0) {
            show_inodes = true;
        } else if (memcmp(argv[i], "-a", 3) == 0) {
            show_private = true;
        } else {
            has_path = true;
        }
    }

    if (has_path) {
        fd = open(argv[1], O_RDONLY | O_DIRECTORY);
    } else {
        fd = open(".", O_RDONLY | O_DIRECTORY);
    }

    if (fd < 0) {
        write(1, "ls: can't open file\n", 20);
        return -1;
    }
    for (;;) {
        nread = getdents(fd, buf, BUF_SIZE);
        if (nread < 0) {
            write(1, "Err\n", 4);
            return -1;
        }

        if (nread == 0)
            break;

        for (bpos = 0; bpos < nread;) {
            d = (struct linux_dirent*)(buf + bpos);
            if (((char*)&d->name)[0] != '.' || show_private) {
                write(1, &d->name, d->name_len + 1);
                if (show_inodes) {
                    write(1, " ", 1);
                    print_int(d->inode);
                }
                write(1, "\n", 1);
            }
            bpos += d->rec_len;
        }
    }
    return 0;
}