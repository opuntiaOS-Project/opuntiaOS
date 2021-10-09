#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    int fd = 0;
    fd = open("/proc/stat", O_RDWR);
    if (fd > 0) {
        TestErr("Opened /proc/stat for write");
    }

    fd = open("/proc/stat", O_RDONLY);
    if (fd <= 0) {
        TestErr("Can't open /proc/stat for read");
    }

    return 0;
}