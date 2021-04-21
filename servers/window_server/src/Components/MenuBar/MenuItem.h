/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../Helpers/TextDrawer.h"
#include "MenuItemAnswer.h"
#include <libfoundation/Logger.h>
#include <libg/Context.h>
#include <libg/Font.h>
#include <string>
#include <vector>

namespace WinServer {

class MenuItem {
public:
    MenuItem(const std::string& title, int id)
        : m_title(title)
        , m_id(id)
    {
    }

    ~MenuItem() = default;

    int id() const { return m_id; }
    const std::string& title() const { return m_title; }

private:
    std::string m_title;
    int m_id { -1 };
};

class MenuDir {
public:
    MenuDir(const std::string& title, int id)
        : m_title(title)
        , m_id(id)
    {
    }

    ~MenuDir() = default;

    inline void set_font(LG::Font& f) { m_font = f; }
    inline void add_item(MenuItem&& item) { m_items.push_back(std::move(item)); }
    inline void add_item(const MenuItem& item) { m_items.push_back(item); }

    inline int id() const { return m_id; }
    inline void set_title(std::string& title) { m_title = title; }
    inline void set_title(std::string&& title) { m_title = std::move(title); }
    inline const std::string& title() const { return m_title; }
    inline const std::vector<MenuItem>& items() const { return m_items; }

    inline size_t width() const { return Helpers::text_width(m_title, m_font); }

    inline void draw_popup(LG::Context& ctx) { }

    [[gnu::always_inline]] inline void draw(LG::Context& ctx)
    {
        ctx.set_fill_color(LG::Color::Black);
        Helpers::draw_text(ctx, { 0, 6 }, m_title, m_font);

        if (m_active) {
            draw_popup(ctx);
        }
    }

    inline MenuItemAnswer click_began(int x, int y)
    {
        m_active = true;
        return MenuItemAnswer::InvalidateMe;
    }

    inline MenuItemAnswer click_ended()
    {
        return MenuItemAnswer::InvalidateMe;
    }

    inline void popup_rect(LG::Rect& r) { }

private:
    int m_id { -1 };
    bool m_active { false };
    std::string m_title;
    LG::Font& m_font { LG::Font::system_font() };
    std::vector<MenuItem> m_items;
};

} // namespace WinServer