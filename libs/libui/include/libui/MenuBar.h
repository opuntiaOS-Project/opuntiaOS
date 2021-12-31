/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../../../../servers/window_server/shared/MessageContent/MenuBar.h"
#include <libui/Common/MenuItem.h>
#include <string>
#include <utility>

namespace UI {

class Window;
class MenuBar {
    friend class Window;

public:
    MenuBar() = default;
    ~MenuBar() = default;

    void add_menu(const Menu&);
    void add_menu(Menu&&);

    std::vector<Menu>& menus() { return m_menus; }
    const std::vector<Menu>& menus() const { return m_menus; }

private:
    void set_host_window_id(int win_id) { m_host_window_id = win_id; }
    int host_window_id() const { return m_host_window_id; }

    int m_host_window_id { -1 };
    std::vector<Menu> m_menus;
};

} // namespace UI