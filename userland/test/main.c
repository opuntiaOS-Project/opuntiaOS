#include <libsystem/syscalls.h>

int acceptsig(int signo)
{
    print(signo);
    return 2;
}

int printf() {
    char buf[256];
    int fd = open("dev/tty0", O_RDWR);
    
    while (1) {
        write(fd, "ucmd> ", 5);
        if (read(fd, buf, 7) < 0) {
            
        }
        write(fd, buf, 0);
    }
}

int main()
{
    // sigaction(8, acceptsig);
    // raise(8);
    printf();
    return 0;
}