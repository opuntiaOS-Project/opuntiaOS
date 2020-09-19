#include <malloc.h>
#include <pthread.h>
#include <string.h>
#include <syscalls.h>

int main()
{
    int fd, i;

    fd = open("/dev/bga", 0);
    if (fd < 0) {
        return 1;
    }

    mmap_params_t mp;
    mp.flags = MAP_SHARED;
    mp.fd = fd;
    mp.size = 1; // ignored
    mp.prot = PROT_READ | PROT_WRITE; // ignored
    uint32_t* screen = (char*)mmap(&mp);
    
    uint32_t clr = 0x000000;
    char back = -1;
    for (;;) {
        for (int i = 0; i < 1024 * 768; i++) {
            screen[i] = 0x00000000 + clr;
        }
        if (clr == 0xFFFFFF) {
            back = -1;
        }
        if (clr == 0x0) {
            back = 1;
        }
        clr += back * 0x10101;
    }
    return 0;
}