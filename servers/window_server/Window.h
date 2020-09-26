#pragma once
#include "../servers/window_server/WSConnection.h"
#include <libcxx/sys/SharedBuffer.h>
#include <sys/types.h>

class Window {
public:
    Window(const CreateWindowMessage& msg);

    void set_buffer(int buffer_id);
    uint32_t width() const { return m_width; }
    uint32_t height() const { return m_height; }
    SharedBuffer<uint32_t>& buffer() { return m_buffer; }
private:
    uint32_t m_width;
    uint32_t m_height;
    SharedBuffer<uint32_t> m_buffer;
};
