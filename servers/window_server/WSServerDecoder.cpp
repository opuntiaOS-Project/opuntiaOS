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
    return new GreetMessageReply(msg.key(), Connection::the().alloc_connection());
}

UniquePtr<Message> WServerDecoder::handle(const CreateWindowMessage& msg)
{
    auto& wm = WindowManager::the();
    int win_id = wm.windows().size();
    wm.add_window(Window(msg.key(), win_id, msg));
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
    auto& wm = WindowManager::the();
    auto& window = wm.window(msg.window_id());
    auto rect = msg.rect();
    rect.offset_by(window.content_bounds().origin());
    rect.intersect(window.content_bounds());
    Compositor::the().invalidate(rect);
    return nullptr;
}

UniquePtr<Message> WServerDecoder::handle(const SetTitleMessage& msg)
{
    auto& wm = WindowManager::the();
    wm.window(msg.window_id()).frame().set_app_name(msg.title());
    return nullptr;
}