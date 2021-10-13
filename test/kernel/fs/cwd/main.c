#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char buf[256];

#define ANSWER "/test_bin/kernel$fs$cwd"

int main(int argc, char** argv)
{
    int fd = 0;
    fd = open("/proc/self/exe", O_RDONLY);
    int rd = read(fd, buf, 256);
    if (rd != strlen(ANSWER)) {
        TestErr("Wrong len");
    }

    if (strncmp(buf, ANSWER, rd)) {
        TestErr("Wrong path");
    }

    return 0;
}