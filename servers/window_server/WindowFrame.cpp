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
    , m_window_control_buttons()
    , m_control_panel_buttons()
{
    auto* close = new Button();
    close->set_icon(LG::GlyphBitmap(s_close_button_glyph_data, 10, 10));
    auto* maximize = new Button();
    maximize->set_icon(LG::GlyphBitmap(s_maximise_button_glyph_data, 10, 10));
    auto* minimize = new Button();
    minimize->set_icon(LG::GlyphBitmap(s_minimise_button_glyph_data, 10, 10));
    
    m_window_control_buttons.push_back(close);
    m_window_control_buttons.push_back(maximize);
    m_window_control_buttons.push_back(minimize);
}

WindowFrame::WindowFrame(Window& window, Vector<Button*>&& control_panel_buttons, Vector<Button*>&& window_control_buttons)
    : m_window(window)
    , m_window_control_buttons(move(window_control_buttons))
    , m_control_panel_buttons(move(control_panel_buttons))
{
}

void WindowFrame::set_app_name(const String& title)
{
    auto* new_control = new Button();
    new_control->set_title(title);
    m_control_panel_buttons.push_back(new_control);
}

void WindowFrame::add_control(const String& title)
{
    auto* new_control = new Button();
    new_control->set_title(title);
    m_control_panel_buttons.push_back(new_control);
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

    int start_controls = x + 8;
    for (int i = 0; i < m_control_panel_buttons.size(); i++) {
        m_control_panel_buttons[i]->display(ctx, { start_controls, y + 8 });
        start_controls += 8 + m_control_panel_buttons[i]->bounds().width();
    }

    int start_buttons = right_x - 8 - m_window_control_buttons[0]->bounds().width();
    for (int i = 0; i < m_window_control_buttons.size(); i++) {
        m_window_control_buttons[i]->display(ctx, { start_buttons, y + 8 });
        start_buttons += - 8 - m_window_control_buttons[i]->bounds().width();
    }
}

void WindowFrame::receive_mouse_event(UniquePtr<MouseEvent> event)
{
}

const LG::Rect WindowFrame::bounds() const
{
    const auto& bounds = m_window.bounds();
    return LG::Rect(bounds.min_x(), bounds.min_y(), bounds.width(), top_border_size());
}