#include "Compositor.h"
#include "Screen.h"
#include <libg/Canvas.h>

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
    auto our_canvas = LG::Canvas(LG::PixelBitmap(Screen::the().write_buffer(), Screen::the().width(), Screen::the().height()));
    for (int i = 0; i < 1024 * 768; i++) {
        Screen::the().write_buffer()[i] = 0x00FeeeeF; // background
    }

    for (int win = 0; win < m_windows.size(); win++) {
        our_canvas.draw(m_windows[win].x(), m_windows[win].y(), m_windows[win].canvas());
    }
    Screen::the().swap_buffers();
}