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

    void receive_mouse_event(UniquePtr<MouseEvent> event);

    void set_app_name(const String& title);
    void set_app_name(String&& title);
    void add_control(const String& title);

    inline Vector<Button*>& window_control_buttons() { return m_window_control_buttons; }
    inline const Vector<Button*>& window_control_buttons() const { return m_window_control_buttons; }

    inline Vector<Button*>& control_panel_buttons() { return m_control_panel_buttons; }
    inline const Vector<Button*>& control_panel_buttons() const { return m_control_panel_buttons; }

    inline LG::Color& color() { return m_color; }
    inline const LG::Color& color() const { return m_color; }
    inline void set_color(const LG::Color& clr) { m_color = clr; }

private:
    int draw_text(LG::Context& ctx, LG::Point<int> pt, const char* text, LG::Font& font);
    Window& m_window;
    Vector<Button*> m_window_control_buttons;
    Vector<Button*> m_control_panel_buttons;
    LG::Color m_color { 0x00342d2d };
};
