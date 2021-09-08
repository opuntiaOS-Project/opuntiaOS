/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../../shared/MessageContent/MenuBar.h"
#include "../Colors.h"
#include "../Components/Elements/Button.h"
#include "../Event.h"
#include <libfoundation/EventReceiver.h>
#include <libg/Context.h>
#include <libg/PixelBitmap.h>

namespace WinServer {
class Compositor;
}

namespace WinServer::Desktop {

class Window;

class WindowFrame {
public:
    explicit WindowFrame(Window& window);
    WindowFrame(Window& window, std::vector<Button*>&& control_panel_buttons, std::vector<Button*>&& window_control_buttons);
    ~WindowFrame() = default;

    void draw(LG::Context&);
    static constexpr size_t std_app_header_size() { return 26; }
    static constexpr size_t std_top_border_frame_size() { return 4; }
    static constexpr size_t std_top_border_size() { return std_top_border_frame_size() + std_app_header_size(); }
    static constexpr size_t std_bottom_border_size() { return 4; }
    static constexpr size_t std_left_border_size() { return 4; }
    static constexpr size_t std_right_border_size() { return 4; }
    inline size_t top_border_size() const { return m_top_border_size; }
    inline size_t bottom_border_size() const { return std_bottom_border_size(); }
    inline size_t left_border_size() const { return std_left_border_size(); }
    inline size_t right_border_size() const { return std_right_border_size(); }

    const LG::Rect bounds() const;

    void receive_tap_event(const LG::Point<int>& tap);

    void set_app_name(const std::string& title);
    void set_app_name(std::string&& title);
    void add_control(const std::string& title);

    inline std::vector<Button*>& window_control_buttons() { return m_window_control_buttons; }
    inline const std::vector<Button*>& window_control_buttons() const { return m_window_control_buttons; }

    inline std::vector<Button*>& control_panel_buttons() { return m_control_panel_buttons; }
    inline const std::vector<Button*>& control_panel_buttons() const { return m_control_panel_buttons; }
    void handle_control_panel_tap(int button_id);

    inline LG::Color& color() { return m_color; }
    inline const LG::Color& color() const { return m_color; }
    inline void set_color(const LG::Color& clr) { m_color = clr; }

    inline TextStyle text_style() const { return m_text_style; }
    void set_text_style(TextStyle ts);

    void set_visible(bool visible)
    {
        m_top_border_size = visible ? std_top_border_size() : 0;
        m_visible = visible;
    }

    bool visible() const { return m_visible; }
    void set_active(bool active) { m_active = active; }
    bool active() const { return m_active; }

    void invalidate(WinServer::Compositor& compositor) const;

    void reload_icon();
    const LG::PixelBitmap& icon() const { return m_icon; }

    static constexpr int spacing() { return 8; }
    static constexpr int icon_width() { return 12; }
    static constexpr int icon_y_offset() { return 7 + std_top_border_frame_size(); }
    static constexpr int text_y_offset() { return 9 + std_top_border_frame_size(); }
    static constexpr int button_y_offset() { return 8 + std_top_border_frame_size(); }

private:
    Window& m_window;
    std::vector<Button*> m_window_control_buttons;
    std::vector<Button*> m_control_panel_buttons;
    LG::Color m_text_colors[2];
    LG::Color m_color { LG::Color::LightSystemBackground };
    LG::PixelBitmap m_icon {};
    size_t m_top_border_size { std_top_border_size() };
    bool m_visible { true };
    bool m_active { true };

    TextStyle m_text_style;
};

} // namespace WinServer