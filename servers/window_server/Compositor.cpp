#include "Compositor.h"
#include "Screen.h"

static Compositor* s_the;

Compositor& Compositor::the()
{
    return *s_the;
}

Compositor::Compositor()
{
    s_the = this;
}

void Compositor::add_window(Window&& window)
{
    m_windows.push_back(move(window));
}

void Compositor::refresh()
{
    for (int i = 0; i < 1024 * 768; i++) {
        Screen::the().write_buffer()[i] = 0x00FeeeeF; // background
    }
    for (int win = 0; win < m_windows.size(); win++) {
        for (int j = 0; j < m_windows[win].width() * m_windows[win].height(); j++) {
            Screen::the().write_buffer()[j] = m_windows[win].buffer()[j];
        }
    }
    Screen::the().swap_buffers();
}