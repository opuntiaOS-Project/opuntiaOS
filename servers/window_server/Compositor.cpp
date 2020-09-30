#include "Compositor.h"
#include "Screen.h"
#include <libg/PixelBitmap.h>

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
        Screen::the().write_bitmap().data()[i] = 0x00FeeeeF; // background
    }

    for (int win = 0; win < m_windows.size(); win++) {
        Screen::the().write_bitmap().draw(m_windows[win].x(), m_windows[win].y(), m_windows[win].bitmap());
    }
    Screen::the().swap_buffers();
}