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
    int win_id = wm.next_win_id();
    auto* window = new Window(msg.key(), win_id, msg);
    window->frame().set_app_name("Unknown app");
    window->set_icon(msg.icon_path());
    wm.add_window(window);
    wm.notify_window_icon_changed(window->id());
    return new CreateWindowMessageReply(msg.key(), win_id);
}

UniquePtr<Message> WServerDecoder::handle(const SetBufferMessage& msg)
{
    auto* window = WindowManager::the().window(msg.window_id());
    if (!window) {
        return nullptr;
    }
    window->set_buffer(msg.buffer_id());
    window->content_bitmap().set_format(LG::PixelBitmapFormat(msg.format()));
    return nullptr;
}

UniquePtr<Message> WServerDecoder::handle(const DestroyWindowMessage& msg)
{
    auto& wm = WindowManager::the();
    auto* window = wm.window(msg.window_id());
    if (window->connection_id() != msg.key()) {
        // TODO: security violation
    }
    wm.remove_window(window);
    return new DestroyWindowMessageReply(msg.key(), 0);
}

UniquePtr<Message> WServerDecoder::handle(const InvalidateMessage& msg)
{
    auto& wm = WindowManager::the();
    auto* window = wm.window(msg.window_id());
    if (!window) {
        return nullptr;
    }
    auto rect = msg.rect();
    rect.offset_by(window->content_bounds().origin());
    rect.intersect(window->content_bounds());
    Compositor::the().invalidate(rect);
    return nullptr;
}

UniquePtr<Message> WServerDecoder::handle(const SetTitleMessage& msg)
{
    auto& wm = WindowManager::the();
    auto* window = wm.window(msg.window_id());
    if (!window) {
        return nullptr;
    }
    window->frame().set_app_name(msg.title());
    return nullptr;
}

UniquePtr<Message> WServerDecoder::handle(const SetBarStyleMessage& msg)
{
    // auto& wm = WindowManager::the();
    // auto* window = wm.window(msg.window_id());
    // if (!window) {
    //     return nullptr;
    // }
    
    return nullptr;
}

UniquePtr<Message> WServerDecoder::handle(const AskBringToFrontMessage& msg)
{
    auto& wm = WindowManager::the();
    auto* window = wm.window(msg.window_id());
    auto* target_window = wm.window(msg.target_window_id());
    if (!window || !target_window) {
        return nullptr;
    }
    if (window->type() == WindowType::Dock) {
        // Only dock can ask for that now.
        wm.bring_to_front(*target_window);
    }
    return nullptr;
}