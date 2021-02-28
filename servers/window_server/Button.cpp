/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Button.h"
#include <algorithm>

void Button::recalc_dims()
{
    size_t new_width = 0;
    size_t new_height = 0;
    if (m_is_icon_set) {
        new_height = m_icon.height();
        new_width += m_icon.width();
        if (m_title.size()) {
            new_width += 4;
        }
    }
    bounds().set_width(new_width + text_width());
    bounds().set_height(std::max(new_height, text_height()));
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