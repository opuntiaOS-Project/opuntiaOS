#include <libsystem/syscalls.h>

int acceptsig(int signo) {
    print(signo);
    return 2;
}

int main() {
    sigaction(9, acceptsig);
    raise(9);
    int tmp = 4;
    while (1) {}
    return tmp;
}