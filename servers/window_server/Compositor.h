#pragma once
#include "WSConnection.h"
#include "WSServerDecoder.h"
#include "Window.h"
#include <libipc/ServerConnection.h>
#include <syscalls.h>

class Compositor {
public:
    static Compositor& the();
    Compositor();

    void add_window(Window&& window);
    Window& window(int id) { return m_windows[0]; } // FIXME!!!
    void refresh();
private:
    Vector<Window>m_windows;
};