#include <libsystem/syscalls.h>

int acceptsig(int signo)
{
    write(1, "oneOS\n", 6);
    return 2;
}

int main()
{
    sigaction(3, acceptsig);
    raise(3);

    char buf[256];
    char* arg[] = {
        "b",
        "a",
        "e",
    };
    arg[2] = (char*)0;

    int sock_fd = socket(PF_LOCAL, 0, 0);
    bind(sock_fd, "ke4.sock", 8);

    int res = fork();
    if (res == 0) {
        execve("/bin/ls", arg, 0);
        return -1;
    }

    while (1) {
        read(sock_fd, buf, 4);
        write(1, buf, 4);
    }
    return 0;
}