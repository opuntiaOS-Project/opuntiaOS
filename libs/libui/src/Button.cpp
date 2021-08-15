/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libfoundation/EventLoop.h>
#include <libui/Button.h>
#include <libui/Context.h>

namespace UI {

Button::Button(View* superview, const LG::Rect& frame)
    : Control(superview, frame)
{
    set_background_color(system_background_color());
}

void Button::display(const LG::Rect& rect)
{
    LG::Context& ctx = graphics_current_context();
    ctx.add_clip(rect);

    ctx.set_fill_color(background_color());
    if (m_button_type == Type::System) {
        if (is_hovered()) {
            ctx.set_fill_color(background_color().darken(8));
        }
    }
    ctx.fill_rounded(bounds(), LG::CornerMask(4));

    size_t content_width = text_width();
    size_t content_height = text_height();
    LG::Point<int> text_start { content_edge_insets().left(), std::max(content_edge_insets().top(), int(bounds().height() - content_height) / 2) };
    if (alignment() == Text::Alignment::Center) {
        text_start.set_x((bounds().width() - content_width) / 2);
    } else if (alignment() == Text::Alignment::Right) {
        text_start.set_x(bounds().width() - content_width);
    }

    auto& f = font();
    const size_t letter_spacing = f.glyph_spacing();

    ctx.set_fill_color(title_color());
    for (int i = 0; i < m_title.size(); i++) {
        ctx.draw(text_start, f.glyph_bitmap(m_title[i]));
        text_start.offset_by(f.glyph_width(m_title[i]) + letter_spacing, 0);
    }
}

void Button::hover_begin(const LG::Point<int>& location)
{
    send_actions(UI::Event::Type::MouseEnterEvent);
    View::hover_begin(location);
}

void Button::hover_end()
{
    send_actions(UI::Event::Type::MouseLeaveEvent);
    View::hover_end();
}

void Button::click_began(const LG::Point<int>& location)
{
    send_actions(UI::Event::Type::MouseDownEvent);
    View::click_began(location);
}

void Button::click_ended()
{
    send_actions(UI::Event::Type::MouseUpEvent);
    View::click_ended();
}

void Button::recalc_bounds()
{
    size_t new_width = text_width() + content_edge_insets().left() + content_edge_insets().right();
    size_t new_height = text_height() + content_edge_insets().top() + content_edge_insets().bottom();
    set_width(new_width);
    set_height(new_height);
}

size_t Button::text_width()
{
    size_t width = 0;
    auto& f = font();
    const size_t letter_spacing = f.glyph_spacing();

    for (int i = 0; i < m_title.size(); i++) {
        width += f.glyph_width(m_title[i]);
    }
    width += std::max(size_t(0), m_title.size() - 1) * letter_spacing;
    return width;
}

size_t Button::text_height() const
{
    return font().glyph_height();
}

} // namespace UI