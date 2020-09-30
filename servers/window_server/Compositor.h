#pragma once
#include "WSConnection.h"
#include "WSServerDecoder.h"
#include "Window.h"
#include <libipc/ServerConnection.h>
#include <std/Vector.h>
#include <syscalls.h>

class Compositor {
public:
    static Compositor& the();
    Compositor();

    void add_window(Window&& window);
    inline Window& window(int id)
    {
        for (int i = 0; i < windows().size(); i++) {
            if (windows()[i].id() == id) {
                return windows()[i];
            }
        }
        return windows()[0];
    }

    inline Vector<Window>& windows() { return m_windows; } // FIXME!!!
    void refresh();

private:
    Vector<Window> m_windows;
};