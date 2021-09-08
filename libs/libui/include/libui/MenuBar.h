/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../../../../servers/window_server/shared/MessageContent/MenuBar.h"
#include <libg/string.h>
#include <utility>

namespace UI {

class Menu;
class MenuBar;

class MenuItem {
    friend class Menu;
    friend class MenuBar;

public:
    MenuItem(LG::string&& title, std::function<void()> action)
        : m_title(title)
        , m_action(action)
    {
    }

    inline const LG::string& title() const { return m_title; }
    inline void invoke() { m_action(); }

private:
    inline void set_id(uint32_t id) { m_id = id; }

    LG::string m_title;
    std::function<void()> m_action;
    uint32_t m_id;
};

class Menu {
    friend class MenuBar;

public:
    explicit Menu(MenuBar& menubar, LG::string&& title)
        : m_menubar(menubar)
        , m_title(std::move(title))
    {
    }

    void add_item(const MenuItem& item);
    inline const LG::string& title() const { return m_title; }
    inline uint32_t menu_id() const { return m_menu_id; }

private:
    inline void set_menu_id(uint32_t id) { m_menu_id = id; }

    MenuBar& m_menubar;
    LG::string m_title;
    uint32_t m_menu_id;
};

class Window;
class MenuBar {
    friend class Window;
    friend class Menu;

public:
    MenuBar() = default;
    ~MenuBar() = default;

    Menu& add_menu(LG::string&& title);

private:
    void set_host_window_id(int win_id) { m_host_window_id = win_id; }
    int host_window_id() const { return m_host_window_id; }
    std::vector<MenuItem>& menu_items() { return m_menu_items; }

    int add_menu_item(const MenuItem& item)
    {
        int id = m_menu_items.size();
        m_menu_items.push_back(item);
        m_menu_items.back().set_id(id);
        return id;
    }

    int m_host_window_id { -1 };
    std::vector<Menu*> m_menus;
    std::vector<MenuItem> m_menu_items;
};

} // namespace UI