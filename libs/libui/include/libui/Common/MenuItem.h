/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <functional>
#include <libipc/VectorEncoder.h>
#include <string>
#include <utility>

namespace UI {

class Menu;
class MenuBar;

class MenuItem {
    friend class Menu;
    friend class MenuBar;

public:
    MenuItem(const std::string& title, std::function<void()> action)
        : m_title(title)
        , m_action(action)
    {
    }

    MenuItem(std::string&& title, std::function<void()> action)
        : m_title(std::move(title))
        , m_action(action)
    {
    }

    inline const std::string& title() const { return m_title; }
    inline void invoke() { m_action(); }

private:
    inline void set_id(uint32_t id) { m_id = id; }

    std::string m_title;
    std::function<void()> m_action;
    uint32_t m_id;
};

class Menu {
    friend class MenuBar;

public:
    Menu() = default;
    Menu(const std::string& title)
        : m_title(title)
    {
    }

    Menu(std::string&& title)
        : m_title(std::move(title))
    {
    }

    inline void add_item(const MenuItem& item)
    {
        int id = m_menu_items.size();
        m_menu_items.push_back(item);
        m_menu_items.back().set_id(id);
    }

    inline const std::string& title() const { return m_title; }
    inline uint32_t menu_id() const { return m_menu_id; }
    std::vector<MenuItem>& items() { return m_menu_items; }
    const std::vector<MenuItem>& items() const { return m_menu_items; }

private:
    inline void set_menu_id(uint32_t id) { m_menu_id = id; }

    std::string m_title {};
    uint32_t m_menu_id;
    std::vector<MenuItem> m_menu_items;
};

}