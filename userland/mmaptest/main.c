#include <sys/mman.h>
#include <unistd.h>

#define BUF_SIZE 512
char buf[BUF_SIZE];

void cat(int fd)
{
    int n = 0;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        if (write(1, buf, n) != n) {
            exit(1);
        }
    }
}

int main(int argc, char** argv)
{
    int fd, i;

    if (argc <= 1) {
        cat(0);
        return 0;
    }

    fd = open(argv[1], 0);
    if (fd < 0) {
        return 1;
    }

    mmap_params_t mp;
    mp.flags = MAP_PRIVATE;
    mp.fd = fd;
    mp.size = 4096;
    mp.prot = PROT_READ;
    char* file = (char*)mmap(&mp);
    if (file) {
        write(1, file, 30);
    }

    if (close(fd) == 0) {
        write(1, "s", 1);
        return 1;
    }

    return 0;
}