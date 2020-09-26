#pragma once
#include "../servers/window_server/WSConnection.h"
#include <libcxx/sys/SharedBuffer.h>
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
    SharedBuffer<uint32_t>& buffer() { return m_buffer; }
    const SharedBuffer<uint32_t>& buffer() const { return m_buffer; }
private:
    uint32_t m_id;
    uint32_t m_width;
    uint32_t m_height;
    SharedBuffer<uint32_t> m_buffer;
};

}