#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char buf[512];

void msg(const char* msg)
{
    printf("[MSG] %s\n", msg);
    fflush(stdout);
}

int main(int argc, char** argv)
{
    int fd;

    fd = open("dirfile", 0);
    if (fd >= 0) {
        msg("create dirfile succeeded");
        return 1;
    }
    fd = open("dirfile", O_CREAT);
    if (chdir("dirfile") == 0) {
        msg("chdir dirfile succeeded");
        return 1;
    }
    if (unlink("dirfile") != 0) {
        msg("unlink dirfile succeeded");
        return 1;
    }

    fd = open(".", O_RDWR);
    if (fd >= 0) {
        msg("open . for writing succeeded");
        return 1;
    }
    fd = open(".", 0);
    if (write(fd, "x", 1) > 0) {
        msg("write . succeeded");
        return 1;
    }
    close(fd);

    return 0;
}