/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "Label.h"
#include "Context.h"
#include <libfoundation/EventLoop.h>
#include <libg/Color.h>
#include <syscalls.h>

namespace UI {

Label::Label(const LG::Rect& frame)
    : View(frame)
{
}

void Label::display(const LG::Rect& rect)
{
    Context ctx(*this);

    LG::Point<int> text_start { content_edge_insets().left(), content_edge_insets().top() };

    auto& f = font();
    ctx.set_fill_color(text_color());
    for (int i = 0; i < m_text.size(); i++) {
        ctx.draw(text_start, f.glyph_bitmap(m_text[i]));
        text_start.offset_by(f.glyph_width(m_text[i]) + 2, 0);
    }
}

void Label::hover_begin(const LG::Point<int>& location)
{
}

void Label::hover_end()
{
}

void Label::recalc_bounds()
{
    size_t new_width = text_width() + content_edge_insets().left() + content_edge_insets().right();
    size_t new_height = text_height() + content_edge_insets().top() + content_edge_insets().bottom();
    set_width(new_width);
    set_height(new_height);
}

size_t Label::text_width() const
{
    size_t width = 0;
    auto& f = font();
    for (int i = 0; i < m_text.size(); i++) {
        width += f.glyph_width(m_text[i]) + 2; // FIXME
    }
    return width;
}

size_t Label::text_height() const
{
    return font().glyph_height();
}

}