#pragma once
#include <syscalls.h>

class Screen {
public:
    static Screen& the();
    Screen();
    void run();
private:
    int m_screen_fd;
    uint32_t* m_screen_buffer;
};