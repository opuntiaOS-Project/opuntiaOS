#include "Window.h"

Window::Window(int id, const CreateWindowMessage& msg)
    : m_id(id)
    , m_buffer(msg.buffer_id())
    , m_width(msg.width())
    , m_height(msg.height())
    , m_canvas()
{
    m_canvas = LG::PixelBitmap(m_buffer.data(), m_width, m_height);
}

void Window::set_buffer(int buffer_id)
{
    m_buffer.open(buffer_id);
}