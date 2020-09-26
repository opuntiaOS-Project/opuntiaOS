#include "WSServerDecoder.h"
#include "Compositor.h"
#include "Window.h"

unique_ptr<Message> WServerDecoder::handle(const GreetMessage& msg)
{
    return new GreetMessageReply(0x1);
}

unique_ptr<Message> WServerDecoder::handle(const CreateWindowMessage& msg)
{
    Compositor::the().add_window(Window(msg));
    return new CreateWindowMessageReply(0x1);
}

unique_ptr<Message> WServerDecoder::handle(const SetBufferMessage& msg)
{
    Compositor::the().window(msg.windows_id()).set_buffer(msg.buffer_id());
    return nullptr;
}