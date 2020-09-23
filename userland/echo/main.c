#include <syscalls.h>

int main(int argc, char** argv)
{
    int pid = fork();
    if (pid == 0) {
        execve("/bin/cpp", 0, 0);
        return 0;
    } else {
        execve("/bin/window_server", 0, 0);
        return 0;
    }
    return 0;
}