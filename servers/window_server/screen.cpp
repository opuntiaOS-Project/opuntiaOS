#include "Screen.h"

static Screen* s_the;

Screen& Screen::the()
{
    return *s_the;
}

Screen::Screen()
{
    s_the = this;
    m_screen_fd = open("/dev/bga", 0);
    mmap_params_t mp;
    mp.flags = MAP_SHARED;
    mp.fd = m_screen_fd;
    mp.size = 1; // ignored
    mp.prot = PROT_READ | PROT_WRITE; // ignored
    m_screen_buffer = (uint32_t*)mmap(&mp);
}

void Screen::run()
{
    uint32_t clr = 0x000000;
    char back = -1;
    for (;;) {
        for (int i = 0; i < 1024 * 768; i++) {
            m_screen_buffer[i] = 0x00000000 + clr;
        }
        write(1, "Refresh\n", 8);
        if (clr == 0x0000FF) {
            back = -1;
        }
        if (clr == 0x0) {
            back = 1;
        }
        clr += back * 0x000001;
    }
}