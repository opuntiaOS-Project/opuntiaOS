#include <libsystem/syscalls.h>

// int acceptsig(int signo)
// {
//     print(signo);
//     return 2;
// }

int main()
{
    char buf[256];
    
    while (1) {
        write(1, "ucmd> ", 5);
        if (read(0, buf, 7) < 0) {
            return -1;
        }
        int fork_res = fork();
        if (fork_res == 0) {
            exec("/bin/echo");
        } else {
            wait(fork_res);
        }
        
    }
    return 0;
}