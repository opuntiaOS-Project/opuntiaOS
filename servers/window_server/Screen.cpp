#include "Screen.h"
#include "Compositor.h"
#include <libcxx/cstring.h>

static Screen* s_the;

Screen& Screen::the()
{
    return *s_the;
}

Screen::Screen()
    : m_width(1024)
    , m_height(768)
    , m_depth(4)
    , m_display_buffer()
    , m_write_buffer()
{
    s_the = this;
    m_screen_fd = open("/dev/bga", 0);
    mmap_params_t mp;
    mp.flags = MAP_SHARED;
    mp.fd = m_screen_fd;
    mp.size = 1; // ignored
    mp.prot = PROT_READ | PROT_WRITE; // ignored

    size_t screen_buffer_size = width() * height() * depth();
    uint32_t* first_buffer = (uint32_t*)mmap(&mp);
    uint32_t* second_buffer = (uint32_t*)((uint8_t*)first_buffer + screen_buffer_size);

    m_display_buffer = (LG::Color*)first_buffer;
    m_write_buffer = (LG::Color*)second_buffer;
    m_active_buffer = 0;
}

void Screen::swap_buffers()
{
    auto tmp_buf = m_display_buffer;
    m_display_buffer = m_write_buffer;
    m_write_buffer = tmp_buf;
    m_active_buffer = !m_active_buffer;
    ioctl(m_screen_fd, BGA_SWAP_BUFFERS, m_active_buffer);
}

void Screen::run()
{
}