#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    int fd = -1;
    fd = open("/boot/kernel.config", O_RDONLY);
    if (fd < 0) {
        TestErr("Can't open kernel.config");
    }

    int dupfd = dup(fd);
    if (dupfd < 0) {
        TestErr("Can't dup file");
    }

    int err = write(dupfd, "abcd", 5);
    if (!err) {
        TestErr("Succesfully write to read only file");
    }

    fstat_t stat;
    if (fstat(fd, &stat) < 0) {
        TestErr("Can't read fstat of orig file");
    }

    fstat_t newstat;
    if (fstat(dupfd, &newstat) < 0) {
        TestErr("Can't read fstat of dup file");
    }

    if (memcmp(&stat, &newstat, sizeof(fstat_t)) != 0) {
        TestErr("Different stat info of files");
    }
    return 0;
}