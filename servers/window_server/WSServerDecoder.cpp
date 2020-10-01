/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "WSServerDecoder.h"
#include "Window.h"
#include "WindowManager.h"

UniquePtr<Message> WServerDecoder::handle(const GreetMessage& msg)
{
    return new GreetMessageReply(0x1);
}

UniquePtr<Message> WServerDecoder::handle(const CreateWindowMessage& msg)
{
    auto& wm = WindowManager::the();
    int win_id = wm.windows().size();
    wm.add_window(Window(win_id, msg));
    return new CreateWindowMessageReply(win_id);
}

UniquePtr<Message> WServerDecoder::handle(const SetBufferMessage& msg)
{
    WindowManager::the().window(msg.windows_id()).set_buffer(msg.buffer_id());
    return nullptr;
}