#pragma once
#include <syscalls.h>

class Screen {
public:
    static Screen& the();
    Screen();

    void swap_buffers();
    void run();
    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }
    uint32_t depth() const { return m_depth; }
    uint32_t* write_buffer() const { return m_write_buffer; }
    uint32_t* display_buffer() const { return m_display_buffer; }
    bool check_buffers();

private:
    int m_screen_fd;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_depth;

    int m_active_buffer;
    uint32_t* m_buffer;
    uint32_t* m_write_buffer; /* Buffer to write */
    uint32_t* m_display_buffer; /* Buffer of the current shown display */
};