/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "ServerDecoder.h"
#include "Components/Security/Violations.h"
#include "Desktop/Window.h"
#include "Mobile/Window.h"
#include "WindowManager.h"

namespace WinServer {

std::unique_ptr<Message> WindowServerDecoder::handle(GreetMessage& msg)
{
    return new GreetMessageReply(msg.key(), Connection::the().alloc_connection());
}

#ifdef TARGET_DESKTOP
std::unique_ptr<Message> WindowServerDecoder::handle(CreateWindowMessage& msg)
{
    auto& wm = WindowManager::the();
    auto& compositor = Compositor::the();
    int win_id = wm.next_win_id();
    auto* window = new Desktop::Window(msg.key(), win_id, msg);
    window->set_app_title(msg.title().move_string());
    window->set_icon_path(msg.icon_path().move_string());
    window->set_style(StatusBarStyle(msg.menubar_style(), msg.color()));
    wm.add_window(window);
    if (window->type() == WindowType::Standard) {
        // After moving windows, we have to invalidate bounds() to make sure that
        // the whole window is rendered (coords are changes after move).
        wm.move_window(window, 8 * win_id, MenuBar::height() + 8 * win_id);
        compositor.invalidate(window->bounds());
    }

    wm.notify_window_icon_changed(window->id());
    wm.notify_window_title_changed(window->id());
    return new CreateWindowMessageReply(msg.key(), win_id);
}
#elif TARGET_MOBILE
std::unique_ptr<Message> WindowServerDecoder::handle(CreateWindowMessage& msg)
{
    auto& wm = WindowManager::the();
    int win_id = wm.next_win_id();
    auto* window = new Mobile::Window(msg.key(), win_id, msg);
    window->set_style(StatusBarStyle(msg.menubar_style(), msg.color()));
    wm.add_window(window);
    wm.notify_window_icon_changed(window->id());
    wm.move_window(window, 0, MenuBar::height());
    return new CreateWindowMessageReply(msg.key(), win_id);
}
#endif

std::unique_ptr<Message> WindowServerDecoder::handle(SetBufferMessage& msg)
{
    auto* window = WindowManager::the().window(msg.window_id());
    if (!window) {
        return nullptr;
    }

    LG::Size new_size = { msg.bounds().width(), msg.bounds().height() };
    window->did_size_change(new_size);
    window->set_buffer(msg.buffer_id(), new_size, LG::PixelBitmapFormat(msg.format()));
    return nullptr;
}

std::unique_ptr<Message> WindowServerDecoder::handle(DestroyWindowMessage& msg)
{
    auto& wm = WindowManager::the();
    auto* window = wm.window(msg.window_id());
    if (!window) {
        wm.on_window_misbehave(*window, ViolationClass::Ignorable);
        return new DestroyWindowMessageReply(msg.key(), 1);
    }

    if (window->connection_id() != msg.key()) {
        wm.on_window_misbehave(*window, ViolationClass::Serious);
        return new DestroyWindowMessageReply(msg.key(), 1);
    }
    wm.remove_window(window);
    return new DestroyWindowMessageReply(msg.key(), 0);
}

std::unique_ptr<Message> WindowServerDecoder::handle(InvalidateMessage& msg)
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
std::unique_ptr<Message> WindowServerDecoder::handle(SetTitleMessage& msg)
{
    auto& wm = WindowManager::the();
    auto* window = wm.window(msg.window_id());
    if (!window) {
        return nullptr;
    }
    window->set_app_title(msg.title().string());

    auto& compositor = Compositor::the();
    compositor.invalidate(compositor.menu_bar().bounds());
    wm.notify_window_title_changed(window->id());
    return nullptr;
}
#elif TARGET_MOBILE
std::unique_ptr<Message> WindowServerDecoder::handle(SetTitleMessage& msg)
{
    return nullptr;
}
#endif

std::unique_ptr<Message> WindowServerDecoder::handle(SetBarStyleMessage& msg)
{
    auto& wm = WindowManager::the();
    auto* window = wm.window(msg.window_id());
    if (!window) {
        return nullptr;
    }

    window->set_style(StatusBarStyle(msg.menubar_style(), msg.color()));
    return nullptr;
}

#ifdef TARGET_DESKTOP
std::unique_ptr<Message> WindowServerDecoder::handle(MenuBarCreateMenuMessage& msg)
{
    auto& wm = WindowManager::the();
    auto* window = wm.window(msg.window_id());
    if (!window) {
        return new MenuBarCreateMenuMessageReply(msg.key(), -1, 0);
    }

    int id = window->menubar_content().size();
    window->menubar_content().push_back(MenuDir(msg.title().string(), id));
    window->on_menubar_change();
    return new MenuBarCreateMenuMessageReply(msg.key(), 0, id);
}

std::unique_ptr<Message> WindowServerDecoder::handle(MenuBarCreateItemMessage& msg)
{
    auto& wm = WindowManager::the();
    auto* window = wm.window(msg.window_id());
    if (!window) {
        return new MenuBarCreateItemMessageReply(msg.key(), -1);
    }

    auto menu_id = msg.menu_id();
    if (menu_id == 0 || window->menubar_content().size() <= menu_id) {
        return new MenuBarCreateItemMessageReply(msg.key(), -2);
    }

    auto callback = [window, menu_id](int item_id) { LFoundation::EventLoop::the().add(Connection::the(),
                                                         new SendEvent(new MenuBarActionMessage(window->connection_id(), window->id(), menu_id, item_id))); };
    window->menubar_content()[menu_id].add_item(PopupItem { msg.item_id(), msg.title().string(), callback });
    // TODO: Currently we don't redraw popup after a new item was added.
    return new MenuBarCreateItemMessageReply(msg.key(), 0);
}
#elif TARGET_MOBILE
std::unique_ptr<Message> WindowServerDecoder::handle(MenuBarCreateMenuMessage& msg)
{
    return new MenuBarCreateMenuMessageReply(msg.key(), -100, 0);
}

std::unique_ptr<Message> WindowServerDecoder::handle(MenuBarCreateItemMessage& msg)
{
    return new MenuBarCreateItemMessageReply(msg.key(), -100);
}
#endif

std::unique_ptr<Message> WindowServerDecoder::handle(PopupShowMenuMessage& msg)
{
    auto& wm = WindowManager::the();
    auto* window = wm.window(msg.window_id());

    if (!window) {
        return new PopupShowMenuMessageReply(msg.key(), -1, -1);
    }

    std::vector<std::string> res;
    for (int i = 0; i < msg.data().vector().size(); i++) {
        res.push_back(msg.data().vector()[i].move_string());
    }

    PopupData popup_data;
    int item_id = 0;
    int menu_id = 0;

    for (auto& title : res) {
        auto callback = [window, menu_id](int item_id) { LFoundation::EventLoop::the().add(Connection::the(),
                                                             new SendEvent(new PopupActionMessage(window->connection_id(), window->id(), menu_id, item_id))); };
        popup_data.push_back(PopupItem { item_id, title, callback });
        item_id++;
    }

    wm.popup().show({ msg.point() }, popup_data);
    return new PopupShowMenuMessageReply(msg.key(), 0, 0);
}

std::unique_ptr<Message> WindowServerDecoder::handle(AskBringToFrontMessage& msg)
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