/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libfoundation/EventLoop.h>
#include <libui/Context.h>
#include <libui/TextField.h>

namespace UI {

TextField::TextField(View* superview, const LG::Rect& frame)
    : Control(superview, frame)
{
    layer().set_corner_mask(LG::CornerMask(4));
    set_background_color(LG::Color::LightSystemButton);
}

void TextField::display(const LG::Rect& rect)
{
    LG::Context& ctx = graphics_current_context();
    ctx.add_clip(rect);

    ctx.set_fill_color(background_color());
    ctx.fill_rounded(bounds(), layer().corner_mask());

    std::string* display_text_ptr;
    if (m_text.empty()) {
        display_text_ptr = &m_placeholder_text;
        ctx.set_fill_color(LG::Color(120, 120, 120));
    } else {
        display_text_ptr = &m_text;
        ctx.set_fill_color(text_color());
    }
    const std::string& display_text = *display_text_ptr;
    size_t content_width = text_width(display_text);
    size_t content_height = text_height(display_text);
    int right_offset_of_last_glyph = std::min(content_edge_insets().left() + content_width, bounds().width() - content_edge_insets().left() - content_edge_insets().right());
    LG::Point<int> last_glyph_start { right_offset_of_last_glyph, std::max(content_edge_insets().top(), int(bounds().height() - content_height) / 2) };

    auto& f = font();
    for (int i = display_text.size() - 1; i >= 0; i--) {
        auto& glyph = f.glyph(display_text[i]);
        last_glyph_start.offset_by(-glyph.advance(), 0);
        ctx.draw(last_glyph_start, glyph);
    }
}

void TextField::mouse_entered(const LG::Point<int>& location)
{
}

void TextField::mouse_exited()
{
}

void TextField::mouse_down(const LG::Point<int>& location)
{
}

void TextField::mouse_up()
{
}

size_t TextField::text_width(const std::string& text)
{
    size_t width = 0;
    auto& f = font();

    for (int i = 0; i < text.size(); i++) {
        auto& glyph = f.glyph(text[i]);
        width += glyph.advance();
    }
    return width;
}

size_t TextField::text_height(const std::string& text) const
{
    return font().size();
}

} // namespace UI