#pragma once
#include <libg/Color.h>
#include <syscalls.h>

class Screen {
public:
    static Screen& the();
    Screen();

    void swap_buffers();
    void run();

    inline uint32_t width() const { return m_width; }
    inline uint32_t height() const { return m_height; }
    inline uint32_t depth() const { return m_depth; }

    inline LG::Color* write_buffer() { return m_write_buffer; }
    inline const LG::Color* write_buffer() const { return m_write_buffer; }
    inline LG::Color* display_buffer() { return m_display_buffer; }
    inline const LG::Color* display_buffer() const { return m_display_buffer; }

private:
    int m_screen_fd;
    uint32_t m_width;
    uint32_t m_height;
    uint32_t m_depth;

    int m_active_buffer;
    LG::Color* m_write_buffer; /* Buffer to write */
    LG::Color* m_display_buffer; /* Buffer of the current shown display */
};