/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "WindowFrame.h"
#include "Button.h"
#include "Window.h"
#include <libg/Font.h>
#include <libg/Rect.h>
#include <std/Utility.h>
#include <syscalls.h>

static const uint32_t s_close_button_glyph_data[10] = {
    0b1100000011,
    0b1110000111,
    0b0111001110,
    0b0011101100,
    0b0001110000,
    0b0000111000,
    0b0011011100,
    0b0111001110,
    0b1110000111,
    0b1100000011,
};

static const uint32_t s_maximise_button_glyph_data[10] = {
    0b1111111111,
    0b1111111111,
    0b1100000011,
    0b1100000011,
    0b1100000011,
    0b1100000011,
    0b1100000011,
    0b1100000011,
    0b1111111111,
    0b1111111111
};

static const uint32_t s_minimise_button_glyph_data[10] = {
    0b0000000000,
    0b0000000000,
    0b0000000000,
    0b0000000000,
    0b0000000000,
    0b0000000000,
    0b0000000000,
    0b0000000000,
    0b1111111111,
    0b1111111111
};

WindowFrame::WindowFrame(Window& window)
    : m_window(window)
{
}

int WindowFrame::draw_text(LG::Context& ctx, LG::Point<int> pt, const char* text, LG::Font& font)
{
    while (*text != 0) {
        ctx.draw(pt, font.glyph_bitmap(*text));
        pt.offset_by(font.glyph_width(*text) + 2, 0);
        text++;
    }
    return pt.x();
}

void WindowFrame::draw(LG::Context& ctx)
{
    int x = m_window.bounds().min_x();
    int y = m_window.bounds().min_y();
    size_t width = m_window.bounds().width();
    size_t height = m_window.bounds().height();

    int right_x = x + width - right_border_size();
    int bottom_y = y + height - bottom_border_size();

    ctx.set_fill_color(LG::Color(std_frame_color()));
    ctx.fill(LG::Rect(x, y, width, top_border_size()));
    ctx.fill(LG::Rect(x, y, left_border_size(), height));

    ctx.fill(LG::Rect(right_x, y, right_border_size(), height));
    ctx.fill(LG::Rect(x, bottom_y, width, bottom_border_size()));

    ctx.set_fill_color(LG::Color::White);

    // FIXME: Just for demo)
    
    Button close;
    close.set_icon(LG::GlyphBitmap(s_close_button_glyph_data, 10, 10));
    int start_buttons = right_x - 8 - close.bounds().width();
    close.display(ctx, { start_buttons, y + 8 });
    Button maximize;
    maximize.set_icon(LG::GlyphBitmap(s_maximise_button_glyph_data, 10, 10));
    start_buttons += - 8 - maximize.bounds().width();
    maximize.display(ctx, { start_buttons, y + 8 });
    Button minimize;
    minimize.set_icon(LG::GlyphBitmap(s_minimise_button_glyph_data, 10, 10));
    start_buttons += - 8 - minimize.bounds().width();
    minimize.display(ctx, { start_buttons, y + 8 });

    int start = draw_text(ctx, { x + 8, y + 8 }, "About", LG::Font::system_bold_font());
    start = draw_text(ctx, { start + 8, y + 8 }, "Menu", LG::Font::system_font());
    start = draw_text(ctx, { start + 8, y + 8 }, "Edit", LG::Font::system_font());
}

void WindowFrame::receive_mouse_event(UniquePtr<MouseEvent> event)
{
}

const LG::Rect WindowFrame::bounds() const
{
    const auto& bounds = m_window.bounds();
    return LG::Rect(bounds.min_x(), bounds.min_y(), bounds.width(), top_border_size());
}