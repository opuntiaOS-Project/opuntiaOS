#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <syscalls.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    if (argc <= 2) {
        return 0;
    }

    printf("%d %d", atoi(argv[1]), atoi(argv[2]));
    kill(atoi(argv[1]), atoi(argv[2]));
    return 0;
}