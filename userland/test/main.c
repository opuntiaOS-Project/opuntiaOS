#include <libsystem/syscalls.h>

int acceptsig(int signo)
{
    print(signo);
    return 2;
}

int main()
{
    sigaction(9, acceptsig);
    raise(9);
    int fd = open("/data.txt", O_RDWR);
    print(fd);
    print(write(fd, "airpods", 8));
    int tmp = 4;
    return tmp;
}