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
    return new GreetMessageReply(msg.key(), 0x1);
}

UniquePtr<Message> WServerDecoder::handle(const CreateWindowMessage& msg)
{
    auto& wm = WindowManager::the();
    int win_id = wm.windows().size();
    wm.add_window(Window(win_id, msg));
    wm.window(win_id).frame().set_app_name("Window");
    return new CreateWindowMessageReply(msg.key(), win_id);
}

UniquePtr<Message> WServerDecoder::handle(const SetBufferMessage& msg)
{
    WindowManager::the().window(msg.window_id()).set_buffer(msg.buffer_id());
    return nullptr;
}

UniquePtr<Message> WServerDecoder::handle(const InvalidateMessage& msg)
{
    // FIXME: Here hust for test :*)
    write(1, "invalidate", 10);
    if (msg.rect().height() == 300) {
        write(1, " ok\n", 4);
    }
    return nullptr;
}

UniquePtr<Message> WServerDecoder::handle(const SetTitleMessage& msg)
{
    auto& wm = WindowManager::the();
    wm.window(msg.window_id()).frame().set_app_name(msg.title());
    return nullptr;
}