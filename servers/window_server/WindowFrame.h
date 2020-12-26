/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once
#include "Button.h"
#include "WSEvent.h"
#include <libfoundation/EventReceiver.h>
#include <libg/Context.h>
#include <libg/PixelBitmap.h>

class Window;

class WindowFrame {
public:
    WindowFrame(Window& window);
    WindowFrame(Window& window, Vector<Button*>&& control_panel_buttons, Vector<Button*>&& window_control_buttons);
    ~WindowFrame() { }

    void draw(LG::Context&);
    constexpr size_t top_border_size() const { return 26; }
    constexpr size_t bottom_border_size() const { return 1; }
    constexpr size_t left_border_size() const { return 1; }
    constexpr size_t right_border_size() const { return 1; }

    const LG::Rect bounds() const;

    void receive_tap_event(const LG::Point<int>& tap);

    void set_app_name(const String& title);
    void set_app_name(String&& title);
    void add_control(const String& title);

    inline Vector<Button*>& window_control_buttons() { return m_window_control_buttons; }
    inline const Vector<Button*>& window_control_buttons() const { return m_window_control_buttons; }

    inline Vector<Button*>& control_panel_buttons() { return m_control_panel_buttons; }
    inline const Vector<Button*>& control_panel_buttons() const { return m_control_panel_buttons; }
    void handle_control_panel_tap(int button_id);

    inline LG::Color& color() { return m_color; }
    inline const LG::Color& color() const { return m_color; }
    inline void set_color(const LG::Color& clr) { m_color = clr; }

    void reload_icon();
    const LG::PixelBitmap& icon() const { return m_icon; }

    static constexpr int spacing() { return 8; }
    static constexpr int icon_y_offset() { return 7; }
    static constexpr int text_y_offset() { return 9; }
    static constexpr int button_y_offset() { return 8; }
    static constexpr int icon_width() { return 12; }

private:
    int draw_text(LG::Context& ctx, LG::Point<int> pt, const char* text, LG::Font& font);
    Window& m_window;
    Vector<Button*> m_window_control_buttons;
    Vector<Button*> m_control_panel_buttons;
    LG::Color m_color { 0x00342d2d };
    LG::PixelBitmap m_icon {};
};
