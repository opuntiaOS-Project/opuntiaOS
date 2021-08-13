/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ServerDecoder.h"
#ifdef TARGET_DESKTOP
#include "Desktop/Window.h"
#elif TARGET_MOBILE
#include "Mobile/Window.h"
#endif
#include "WindowManager.h"

namespace WinServer {

std::unique_ptr<Message> WindowServerDecoder::handle(const GreetMessage& msg)
{
    return new GreetMessageReply(msg.key(), Connection::the().alloc_connection());
}

#ifdef TARGET_DESKTOP
std::unique_ptr<Message> WindowServerDecoder::handle(const CreateWindowMessage& msg)
{
    auto& wm = WindowManager::the();
    int win_id = wm.next_win_id();
    auto* window = new Desktop::Window(msg.key(), win_id, msg);
    window->set_name("App");
    window->set_icon(msg.icon_path());
    wm.add_window(window);
    if (window->type() == WindowType::Standard) {
        wm.move_window(window, 8 * win_id, MenuBar::height() + 8 * win_id);
    }
    wm.notify_window_icon_changed(window->id());
    return new CreateWindowMessageReply(msg.key(), win_id);
}
#elif TARGET_MOBILE
std::unique_ptr<Message> WindowServerDecoder::handle(const CreateWindowMessage& msg)
{
    auto& wm = WindowManager::the();
    int win_id = wm.next_win_id();
    auto* window = new Mobile::Window(msg.key(), win_id, msg);
    wm.add_window(window);
    wm.notify_window_icon_changed(window->id());
    wm.move_window(window, 0, MenuBar::height());
    return new CreateWindowMessageReply(msg.key(), win_id);
}
#endif

std::unique_ptr<Message> WindowServerDecoder::handle(const SetBufferMessage& msg)
{
    auto* window = WindowManager::the().window(msg.window_id());
    if (!window) {
        return nullptr;
    }
    window->set_buffer(msg.buffer_id());
    window->content_bitmap().set_format(LG::PixelBitmapFormat(msg.format()));
    return nullptr;
}

std::unique_ptr<Message> WindowServerDecoder::handle(const DestroyWindowMessage& msg)
{
    auto& wm = WindowManager::the();
    auto* window = wm.window(msg.window_id());
    if (window->connection_id() != msg.key()) {
        // TODO: security violation
        return new DestroyWindowMessageReply(msg.key(), 1);
    }
    wm.remove_window(window);
    return new DestroyWindowMessageReply(msg.key(), 0);
}

std::unique_ptr<Message> WindowServerDecoder::handle(const InvalidateMessage& msg)
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

#ifdef TARGET_DESKTOP
std::unique_ptr<Message> WindowServerDecoder::handle(const SetTitleMessage& msg)
{
    auto& wm = WindowManager::the();
    auto* window = wm.window(msg.window_id());
    if (!window) {
        return nullptr;
    }
    window->set_name(msg.title());

    auto& compositor = Compositor::the();
    compositor.invalidate(compositor.menu_bar().bounds());
    return nullptr;
}
#elif TARGET_MOBILE
std::unique_ptr<Message> WindowServerDecoder::handle(const SetTitleMessage& msg)
{
    return nullptr;
}
#endif

std::unique_ptr<Message> WindowServerDecoder::handle(const SetBarStyleMessage& msg)
{
    // auto& wm = WindowManager::the();
    // auto* window = wm.window(msg.window_id());
    // if (!window) {
    //     return nullptr;
    // }

    return nullptr;
}

#ifdef TARGET_DESKTOP
std::unique_ptr<Message> WindowServerDecoder::handle(const MenuBarCreateMenuMessage& msg)
{
    auto& wm = WindowManager::the();
    auto* window = wm.window(msg.window_id());
    if (!window) {
        return new MenuBarCreateMenuMessageReply(msg.key(), -1, 0);
    }

    int id = window->menubar_content().size();
    window->menubar_content().push_back(MenuDir(msg.title(), id));
    return new MenuBarCreateMenuMessageReply(msg.key(), 0, id);
}

std::unique_ptr<Message> WindowServerDecoder::handle(const MenuBarCreateItemMessage& msg)
{
    auto& wm = WindowManager::the();
    auto* window = wm.window(msg.window_id());
    if (!window) {
        return new MenuBarCreateItemMessageReply(msg.key(), -1);
    }

    if (msg.menu_id() == 0 || window->menubar_content().size() <= msg.menu_id()) {
        return new MenuBarCreateItemMessageReply(msg.key(), -2);
    }

    auto callback = [window](int item_id) { LFoundation::EventLoop::the().add(Connection::the(),
                                                new SendEvent(new MenuBarActionMessage(window->connection_id(), window->id(), item_id))); };
    window->menubar_content()[msg.menu_id()].add_item(PopupItem { msg.item_id(), msg.title(), callback });
    return new MenuBarCreateItemMessageReply(msg.key(), 0);
}
#elif TARGET_MOBILE
std::unique_ptr<Message> WindowServerDecoder::handle(const MenuBarCreateMenuMessage& msg)
{
    return new MenuBarCreateMenuMessageReply(msg.key(), -100, 0);
}

std::unique_ptr<Message> WindowServerDecoder::handle(const MenuBarCreateItemMessage& msg)
{
    return new MenuBarCreateMenuMessageReply(msg.key(), -100, 0);
}
#endif

std::unique_ptr<Message> WindowServerDecoder::handle(const AskBringToFrontMessage& msg)
{
    auto& wm = WindowManager::the();
    auto* window = wm.window(msg.window_id());
    auto* target_window = wm.window(msg.target_window_id());
    if (!window || !target_window) {
        return nullptr;
    }
    if (window->type() == WindowType::Homescreen) {
        // Only dock can ask for that now.
        wm.ask_to_set_active_window(*target_window);
    }
    return nullptr;
}

} // namespace WinServer