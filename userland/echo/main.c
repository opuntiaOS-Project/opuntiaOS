#include <libsystem/syscalls.h>

int main(int argc, char** argv)
{
    int res = fork();

    int sock_fd = socket(PF_LOCAL, 0, 0);
    res = connect(sock_fd, "ke4.sock", 8);
    if (res < 0) {
        return -1;
    }

    while (1) {
        if (res)
            write(sock_fd, "icu\n", 4);
        else 
            write(sock_fd, "ufc\n", 4);
        
        // Sleep)
        for (int i = 0; i < 100000000; i++) {
            int a = 3;
            a += 555;
            a += 33;
            i -= 33;
            i += a;
            i -= 555;
        }
    }

    return 0;
}