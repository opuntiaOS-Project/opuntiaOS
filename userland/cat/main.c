#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <syscalls.h>
#include <unistd.h>

#define BUF_SIZE 512
char buf[BUF_SIZE];

void cat(int fd)
{
    int n = 0;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        if (fwrite(buf, n, 1, stdout) != n) {
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

    for (i = 1; i < argc; i++) {
        if ((fd = open(argv[i], 0)) < 0) {
            // printf(1, "cat: cannot open %s\n", argv[i]);
            return 1;
        }
        cat(fd);
        close(fd);
    }
    return 0;
}