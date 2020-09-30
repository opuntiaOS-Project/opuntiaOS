#include "WSServerDecoder.h"
#include "Compositor.h"
#include "Window.h"

UniquePtr<Message> WServerDecoder::handle(const GreetMessage& msg)
{
    return new GreetMessageReply(0x1);
}

UniquePtr<Message> WServerDecoder::handle(const CreateWindowMessage& msg)
{
    int win_id = Compositor::the().windows().size();
    Compositor::the().add_window(Window(win_id, msg));
    return new CreateWindowMessageReply(win_id);
}

UniquePtr<Message> WServerDecoder::handle(const SetBufferMessage& msg)
{
    Compositor::the().window(msg.windows_id()).set_buffer(msg.buffer_id());
    return nullptr;
}