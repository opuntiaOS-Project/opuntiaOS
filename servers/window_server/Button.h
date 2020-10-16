/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once

#include <libg/Color.h>
#include <libg/Context.h>
#include <std/String.h>

class Button {
public:
    Button() = default;
    ~Button() { }

    void display();
    void set_title(const String& title) { m_title = title, recalc_width(); }
    void set_title(String&& title) { m_title = move(title), recalc_width(); }
    const String& title() const { return m_title; }

    // void set_font(const LG::Font& font) { m_is_font_set = true, m_font = font, recalc_width(); }
    void set_icon(const LG::GlyphBitmap& icon) { m_is_icon_set = true, m_icon = icon, recalc_width(); }

    void set_title_color(const LG::Color& color) { m_title_color = color; }
    const LG::Color& title_color() const { return m_title_color; }

    // const LG::Font& font() const { return m_is_font_set ? m_font : LG::Font::system_font(); }
    inline const LG::Font& font() const { return LG::Font::system_font(); }

    inline LG::Rect& bounds() { return m_bounds; }
    inline const LG::Rect& bounds() const { return m_bounds; }

    void display(LG::Context& ctx, LG::Point<int> pt);

private:
    void recalc_width();
    size_t text_width();

    LG::Rect m_bounds {};
    String m_title {};
    // LG::Font m_font;
    LG::Color m_title_color;
    LG::GlyphBitmap m_icon;

    bool m_is_font_set { false };
    bool m_is_icon_set { false };
};