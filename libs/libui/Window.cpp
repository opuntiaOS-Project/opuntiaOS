#include "Window.h"
#include "WindowConnection.h"

namespace Window {

Window::Window()
    : m_width(400)
    , m_height(300)
    , m_buffer(size_t(m_width * m_height * 4))
{
    m_id = WindowConnection::the().new_window(*this);
}

Window::Window(uint32_t width, uint32_t height)
    : m_width(width)
    , m_height(height)
    , m_buffer(size_t(m_width * m_height * 4))
{
    m_id = WindowConnection::the().new_window(*this);
}

// test
void Window::run()
{
    uint32_t clr = 0x000000;
    char back = -1;
    for (;;) {
        for (int i = 0; i < m_width * m_height; i++) {
            m_buffer[i] = 0x00000000 + clr;
        }
        if (clr == 0x0000FF) {
            back = -1;
        }
        if (clr == 0x0) {
            back = 1;
        }
        clr += back * 0x000001;
    }
}

}