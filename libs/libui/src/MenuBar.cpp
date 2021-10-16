/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libfoundation/EventLoop.h>
#include <libui/App.h>
#include <libui/MenuBar.h>

namespace UI {

void MenuBar::add_menu(const Menu& menu)
{
    Menu new_menu = menu;
    auto& connection = App::the().connection();
    auto resp_message = connection.send_sync_message<MenuBarCreateMenuMessageReply>(MenuBarCreateMenuMessage(connection.key(), m_host_window_id, new_menu.title()));
    new_menu.set_menu_id(resp_message->menu_id());

    // TODO: Speed up sending only 1 message.
    for (auto& item : new_menu.items()) {
        auto resp_message = connection.send_sync_message<MenuBarCreateItemMessageReply>(MenuBarCreateItemMessage(connection.key(), host_window_id(), new_menu.menu_id(), item.m_id, item.title()));
    }

    m_menus.push_back(std::move(new_menu));
}

void MenuBar::add_menu(Menu&& menu)
{
    Menu new_menu = std::move(menu);
    auto& connection = App::the().connection();
    auto resp_message = connection.send_sync_message<MenuBarCreateMenuMessageReply>(MenuBarCreateMenuMessage(connection.key(), m_host_window_id, new_menu.title()));
    new_menu.set_menu_id(resp_message->menu_id());

    // TODO: Speed up sending only 1 message.
    for (auto& item : new_menu.items()) {
        auto resp_message = connection.send_sync_message<MenuBarCreateItemMessageReply>(MenuBarCreateItemMessage(connection.key(), host_window_id(), new_menu.menu_id(), item.m_id, item.title()));
    }

    m_menus.push_back(std::move(new_menu));
}

} // namespace UI