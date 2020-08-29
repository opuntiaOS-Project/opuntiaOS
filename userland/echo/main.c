#include <libsystem/syscalls.h>

int main(int argc, char** argv)
{
    int sock_fd = socket(PF_LOCAL, 0, 0);
    connect(sock_fd, "ke4.sock", 8);

    while (1) {
        write(sock_fd, "icu\n", 4);
        
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