/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libfoundation/EventLoop.h>
#include <libg/Point.h>
#include <libipc/VectorEncoder.h>
#include <libui/App.h>
#include <libui/PopupMenu.h>

namespace UI {

void PopupMenu::show(LG::Point<int> point, const Menu& menu)
{
    m_menu = menu;
    auto& connection = App::the().connection();

    std::vector<LIPC::StringEncoder> tmp;
    for (auto& item : m_menu.items()) {
        tmp.push_back(item.title());
    }

    auto resp_message = connection.send_sync_message<PopupShowMenuMessageReply>(PopupShowMenuMessage(connection.key(), m_host_window_id, point, LIPC::VectorEncoder<LIPC::StringEncoder>(std::move(tmp))));
}

void PopupMenu::show(LG::Point<int> point, Menu&& menu)
{
    m_menu = std::move(menu);
    auto& connection = App::the().connection();

    std::vector<LIPC::StringEncoder> tmp;
    for (auto& item : m_menu.items()) {
        tmp.push_back(item.title());
    }

    auto resp_message = connection.send_sync_message<PopupShowMenuMessageReply>(PopupShowMenuMessage(connection.key(), m_host_window_id, point, LIPC::VectorEncoder<LIPC::StringEncoder>(std::move(tmp))));
}

} // namespace UI