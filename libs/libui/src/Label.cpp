/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libfoundation/EventLoop.h>
#include <libg/Color.h>
#include <libui/Context.h>
#include <libui/Label.h>

namespace UI {

Label::Label(View* superview, const LG::Rect& frame)
    : View(superview, frame)
{
}

Label::Label(View* superview, Window* window, const LG::Rect& frame)
    : View(superview, window, frame)
{
}

void Label::display(const LG::Rect& rect)
{
    LG::Context ctx = graphics_current_context();
    ctx.add_clip(rect);

    auto& f = font();
    const size_t letter_spacing = f.glyph_spacing();

    size_t label_width = bounds().width() - content_edge_insets().left() - content_edge_insets().right();
    size_t txt_width = text_width();
    size_t dots_width = font().glyph_width('.') * 3 + letter_spacing * 2;

    bool need_to_stop_rendering_text = (txt_width > label_width);
    size_t width_when_stop_rendering_text = content_edge_insets().left() + label_width - dots_width;

    size_t content_width = text_width();
    size_t content_height = text_height();
    LG::Point<int> text_start { content_edge_insets().left(), std::max(content_edge_insets().top(), int(bounds().height() - content_height) / 2) };
    if (alignment() == Text::Alignment::Center) {
        int preffered_start = ((int)bounds().width() - (int)content_width) / 2;
        text_start.set_x(std::max(content_edge_insets().left(), preffered_start));
    } else if (alignment() == Text::Alignment::Right) {
        int preffered_start = (int)bounds().width() - (int)content_width;
        text_start.set_x(std::max(content_edge_insets().left(), preffered_start));
    }

    ctx.set_fill_color(text_color());
    for (int i = 0; i < m_text.size(); i++) {
        size_t glyph_width = f.glyph_width(m_text[i]) + letter_spacing;
        if (need_to_stop_rendering_text && text_start.x() + glyph_width > width_when_stop_rendering_text) {
            for (int j = 0; j < 3; j++) {
                ctx.draw(text_start, f.glyph_bitmap('.'));
                text_start.offset_by(f.glyph_width('.') + letter_spacing, 0);
            }
            return;
        }
        ctx.draw(text_start, f.glyph_bitmap(m_text[i]));
        text_start.offset_by(glyph_width, 0);
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
    if (!m_text.size()) {
        return 0;
    }

    size_t width = 0;
    auto& f = font();
    const size_t letter_spacing = f.glyph_spacing();

    for (int i = 0; i < m_text.size(); i++) {
        width += f.glyph_width(m_text[i]) + letter_spacing;
    }
    return width - letter_spacing;
}

size_t Label::text_height() const
{
    return font().glyph_height();
}

} // namespace UI