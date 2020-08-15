#include <libsystem/syscalls.h>

int main()
{
    char buf[256];
    if (read(0, buf, 7) < 0) {
        return -1;    
    }
    write(1, buf, 0);
    return 0;
}