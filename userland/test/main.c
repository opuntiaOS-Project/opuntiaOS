#include <libsystem/syscalls.h>

int acceptsig(int signo)
{
    print(signo);
    return 2;
}

int main()
{
    sigaction(8, acceptsig);
    raise(8);
    int fd = open("/data.txt", O_RDWR);
    if (write(fd, "airpods", 8) < 0) {
        return 1;
    }
    return 0;
}