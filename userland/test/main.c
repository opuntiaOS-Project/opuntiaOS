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

    while (1) {
        write(1, "ucmd> ", 5);
        if (read(0, buf, 7) < 0) {
            return -1;
        }
        int fork_res = fork();
        if (fork_res == 0) {
            execve("/bin/echo", arg, 0);
            return 0;
        } else {
            wait(fork_res);
        }
    }
    return 0;
}