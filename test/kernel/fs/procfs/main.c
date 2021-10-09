#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void msg(const char* msg)
{
    printf("[MSG] %s\n", msg);
    fflush(stdout);
}

int main(int argc, char** argv)
{
    int fd = 0;
    fd = open("/proc/stat", O_RDWR);
    if (fd > 0) {
        msg("Opened /proc/stat for write");
        return 1;
    }

    fd = open("/proc/stat", O_RDONLY);
    if (fd <= 0) {
        msg("Can't open /proc/stat for read");
        return 1;
    }

    return 0;
}