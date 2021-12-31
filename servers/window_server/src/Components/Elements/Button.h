/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <libg/Color.h>
#include <libg/Context.h>
#include <string>

namespace WinServer {

class Button {
public:
    Button() = default;
    ~Button() = default;

    void display();
    void set_title(const std::string& title) { m_title = title, recalc_dims(); }
    void set_title(std::string&& title) { m_title = std::move(title), recalc_dims(); }
    const std::string& title() const { return m_title; }

    void set_font(const LG::Font& font) { m_font = font, recalc_dims(); }
    void set_icon(const LG::GlyphBitmap& icon) { m_is_icon_set = true, m_icon = icon, recalc_dims(); }

    void set_title_color(const LG::Color& color) { m_title_color = color; }
    const LG::Color& title_color() const { return m_title_color; }

    inline const LG::Font& font() const { return m_font; }

    inline LG::Rect& bounds() { return m_bounds; }
    inline const LG::Rect& bounds() const { return m_bounds; }

    void display(LG::Context& ctx, LG::Point<int> pt);

private:
    void recalc_dims();
    size_t text_width();
    inline size_t text_height() const { return font().glyph_height(); }

    LG::Rect m_bounds {};
    std::string m_title {};
    LG::Font m_font { LG::Font::system_font() };
    LG::Color m_title_color;
    LG::GlyphBitmap m_icon;

    bool m_is_icon_set { false };
};

} // namespace WinServer