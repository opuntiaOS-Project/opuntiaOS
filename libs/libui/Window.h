#pragma once
#include "../servers/window_server/WSConnection.h"
#include <libcxx/sys/SharedBuffer.h>
#include <libg/Color.h>
#include <sys/types.h>

namespace Window {

class Window {
public:
    Window();
    Window(uint32_t width, uint32_t height);

    int id() const { return m_id; }
    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }

    void run();
    SharedBuffer<LG::Color>& buffer() { return m_buffer; }
    const SharedBuffer<LG::Color>& buffer() const { return m_buffer; }

private:
    uint32_t m_id;
    uint32_t m_width;
    uint32_t m_height;
    SharedBuffer<LG::Color> m_buffer;
};

}