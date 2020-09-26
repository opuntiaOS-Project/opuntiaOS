#include "Window.h"

Window::Window(const CreateWindowMessage& msg)
    : m_width(msg.width())
    , m_height(msg.height())
{
}

void Window::set_buffer(int buffer_id)
{
    m_buffer.open(buffer_id);
}