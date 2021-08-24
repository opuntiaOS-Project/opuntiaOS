/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libg/Color.h>
#include <libui/Context.h>
#include <libui/TextView.h>
#include <utility>

namespace UI {

TextView::TextView(View* superview, const LG::Rect& frame)
    : ScrollView(superview, frame)
{
}

TextView::TextView(View* superview, Window* window, const LG::Rect& frame)
    : ScrollView(superview, window, frame)
{
}

void TextView::display(const LG::Rect& rect)
{
    LG::Context ctx = graphics_current_context();
    ctx.add_clip(rect);

    auto& f = font();
    const size_t letter_spacing = f.glyph_spacing();
    const size_t line_height = f.glyph_height();

    int start_x = -content_offset().x();
    int start_y = -content_offset().y();
    int cur_x = start_x;
    int cur_y = start_y;

    for (int i = 0; i < m_text.size(); i++) {
        if (m_text[i] == '\n') {
            cur_x = start_x;
            cur_y += line_height;
            continue;
        }

        size_t glyph_width = f.glyph_width(m_text[i]) + letter_spacing;
        if (bounds().contains(cur_x, cur_y) || bounds().contains(cur_x + glyph_width, cur_y + line_height)) {
            ctx.draw({ cur_x, cur_y }, f.glyph_bitmap(m_text[i]));
        }
        cur_x += glyph_width;
    }

    display_scroll_indicators(ctx);
}

void TextView::mouse_entered(const LG::Point<int>& location)
{
    set_hovered(true);
}

void TextView::mouse_exited()
{
    set_hovered(false);
}

void TextView::recalc_text_size()
{
    const size_t letter_spacing = font().glyph_spacing();
    const size_t line_height = font().glyph_height();

    int cur_x = 0;
    int max_x = 0;
    int cur_y = 0;

    for (int i = 0; i < m_text.size(); i++) {
        if (m_text[i] == '\n') {
            std::max(max_x, cur_x);
            cur_x = 0;
            cur_y += line_height;
            continue;
        }

        size_t glyph_width = font().glyph_width(m_text[i]) + letter_spacing;
        cur_x += glyph_width;
    }

    std::max(max_x, cur_x);
    content_size().set(LG::Size(cur_x, cur_y + line_height));
}

} // namespace UI