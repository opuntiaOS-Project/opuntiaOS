/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
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
    layer().set_corner_mask(LG::CornerMask(4));
    set_background_color(LG::Color::LightSystemButton);
}

void Button::display(const LG::Rect& rect)
{
    LG::Context& ctx = graphics_current_context();
    ctx.add_clip(rect);

    ctx.set_fill_color(background_color());
    if (m_button_type == Type::System) {
        if (is_hovered()) {
            ctx.set_fill_color(background_color().darken(5));
        }
    }
    ctx.fill_rounded(bounds(), layer().corner_mask());

    size_t content_width = text_width();
    size_t content_height = text_height();
    LG::Point<int> text_start { content_edge_insets().left(), std::max(content_edge_insets().top(), int(bounds().height() - content_height) / 2) };
    if (alignment() == Text::Alignment::Center) {
        text_start.set_x((bounds().width() - content_width) / 2);
    } else if (alignment() == Text::Alignment::Right) {
        text_start.set_x(bounds().width() - content_width);
    }

    auto& f = font();
    ctx.set_fill_color(title_color());
    for (int i = 0; i < m_title.size(); i++) {
        auto& glyph = f.glyph(m_title[i]);
        ctx.draw(text_start, glyph);
        text_start.offset_by(glyph.advance(), 0);
    }
}

void Button::mouse_entered(const LG::Point<int>& location)
{
    send_actions(UI::Event::Type::MouseEnterEvent);
    View::mouse_entered(location);
    set_needs_display();
}

void Button::mouse_exited()
{
    send_actions(UI::Event::Type::MouseLeaveEvent);
    View::mouse_exited();
    set_needs_display();
}

void Button::mouse_down(const LG::Point<int>& location)
{
    send_actions(UI::Event::Type::MouseDownEvent);
    View::mouse_down(location);
}

void Button::mouse_up()
{
    send_actions(UI::Event::Type::MouseUpEvent);
    View::mouse_up();
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

    for (int i = 0; i < m_title.size(); i++) {
        auto& glyph = f.glyph(m_title[i]);
        width += glyph.advance();
    }
    return width;
}

size_t Button::text_height() const
{
    return font().size();
}

} // namespace UI