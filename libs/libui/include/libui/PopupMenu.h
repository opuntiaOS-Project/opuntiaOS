/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libui/Common/MenuItem.h>
#include <string>
#include <utility>

namespace UI {

class Window;
class PopupMenu {
    friend class Window;

public:
    PopupMenu() = default;
    ~PopupMenu() = default;

    void show(LG::Point<int>, const Menu& menu);
    void show(LG::Point<int>, Menu&& menu);

    Menu& menu() { return m_menu; }
    const Menu& menu() const { return m_menu; }

private:
    void set_host_window_id(int win_id) { m_host_window_id = win_id; }

    Menu m_menu;
    uint32_t m_host_window_id;
};

} // namespace UI