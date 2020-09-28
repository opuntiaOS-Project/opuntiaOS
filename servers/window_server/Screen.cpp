#include "Screen.h"
#include "Compositor.h"
#include <cstring.h>

static Screen* s_the;

Screen& Screen::the()
{
    return *s_the;
}

Screen::Screen()
    : m_width(1024)
    , m_height(768)
    , m_depth(4)
{
    s_the = this;
    m_screen_fd = open("/dev/bga", 0);
    mmap_params_t mp;
    mp.flags = MAP_SHARED;
    mp.fd = m_screen_fd;
    mp.size = 1; // ignored
    mp.prot = PROT_READ | PROT_WRITE; // ignored
    m_buffer = (uint32_t*)mmap(&mp);

    m_display_buffer = m_buffer;
    m_write_buffer = (uint32_t*)((uint8_t*)m_buffer + width() * height() * depth());
    m_active_buffer = 0;
}

void Screen::swap_buffers()
{
    uint32_t* tmp_buf = m_display_buffer;
    m_display_buffer = m_write_buffer;
    m_write_buffer = tmp_buf;
    m_active_buffer = !m_active_buffer;
    ioctl(m_screen_fd, BGA_SWAP_BUFFERS, m_active_buffer);
}

void Screen::run()
{
}