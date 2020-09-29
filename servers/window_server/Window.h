#pragma once
#include "../servers/window_server/WSConnection.h"
#include <libcxx/sys/SharedBuffer.h>
#include <libg/Canvas.h>
#include <libg/Rect.h>
#include <sys/types.h>

class Window {
public:
    Window(int id, const CreateWindowMessage& msg);

    void set_buffer(int buffer_id);

    inline int id() const { return m_id; }
    inline int x() const { return m_x + 500 * m_id; } // REMOVE
    inline int y() const { return m_y + 200 * m_id; } // REMOVE
    inline size_t width() const { return m_width; }
    inline size_t height() const { return m_height; }

    inline SharedBuffer<LG::Color>& buffer() { return m_buffer; }
    inline const LG::Canvas& canvas() const { return m_canvas; }

    LG::Rect bounds() const { return LG::Rect(m_x, m_y, m_width, m_height); }

private:
    int m_id { -1 };
    int m_x { 0 };
    int m_y { 0 };
    size_t m_width;
    size_t m_height;
    LG::Canvas m_canvas;
    SharedBuffer<LG::Color> m_buffer;
};
