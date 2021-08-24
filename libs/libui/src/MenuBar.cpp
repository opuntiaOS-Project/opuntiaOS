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

void Menu::add_item(const MenuItem& item)
{
    int host_window_id = m_menubar.host_window_id();
    int item_id = m_menubar.add_menu_item(item);
    auto& connection = App::the().connection();
    auto resp_message = connection.send_sync_message<MenuBarCreateItemMessageReply>(MenuBarCreateItemMessage(connection.key(), host_window_id, menu_id(), item_id, item.title()));
}

Menu& MenuBar::add_menu(LG::string&& title)
{
    Menu* new_menu = new Menu(*this, std::move(title));
    auto& connection = App::the().connection();
    auto resp_message = connection.send_sync_message<MenuBarCreateMenuMessageReply>(MenuBarCreateMenuMessage(connection.key(), m_host_window_id, new_menu->title()));
    new_menu->set_menu_id(resp_message->menu_id());
    m_menus.push_back(new_menu);
    return *new_menu;
}

} // namespace UI