#pragma once
#include <syscalls.h>

class Screen {
public:
    static Screen& the();
    Screen();
    void run();
    uint32_t* buffer() { return m_screen_buffer; }
private:
    int m_screen_fd;
    uint32_t* m_screen_buffer;
};