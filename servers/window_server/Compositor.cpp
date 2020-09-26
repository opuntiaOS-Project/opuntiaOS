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
        Screen::the().buffer()[i] = 0x00FFFFFF; // background
    }

    for (int i = 0; i < m_windows.size(); i++) {
        for (int j = 0; j < m_windows[i].width() * m_windows[i].height(); j++) {
            Screen::the().buffer()[j] = m_windows[i].buffer()[j]; // app screen
        }
    }

}