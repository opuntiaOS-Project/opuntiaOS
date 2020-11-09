/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "Button.h"

void Button::recalc_width()
{
    size_t new_width = 0;
    if (m_is_icon_set) {
        new_width += m_icon.width();
        if (m_title.size()) {
            new_width += 4;  
        }
    }
    bounds().set_width(new_width);
}

size_t Button::text_width()
{
    size_t width = 0;
    auto& f = font();
    for (int i = 0; i < m_title.size(); i++) {
        width += f.glyph_width(m_title[i]) + 2; // FIXME
    }
    return width;
}

void Button::display(LG::Context& ctx, LG::Point<int> pt)
{
    if (m_is_icon_set) {
        ctx.draw(pt, m_icon);
        pt.offset_by(m_icon.width() + 4, 0);
    }

    auto& f = font();
    for (int i = 0; i < m_title.size(); i++) {
        ctx.draw(pt, f.glyph_bitmap(m_title[i]));
        pt.offset_by(f.glyph_width(m_title[i]) + 2, 0);
    }
}