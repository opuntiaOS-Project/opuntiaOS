/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libg/Font.h>
#include <libui/Control.h>
#include <libui/EdgeInsets.h>
#include <string>

namespace UI {

class Button : public Control {
    UI_OBJECT();

public:
    enum Type {
        System,
        Custom,
    };

    ~Button() = default;

    const std::string& title() const { return m_title; }
    void set_title(const std::string& title) { m_title = title, recalc_bounds(), set_needs_display(); }
    void set_title(std::string&& title) { m_title = std::move(title), recalc_bounds(), set_needs_display(); }

    void set_title_color(const LG::Color& color) { m_title_color = color; }
    const LG::Color& title_color() const { return m_title_color; }

    void set_content_edge_insets(const EdgeInsets& ei) { m_content_edge_insets = ei; }
    const EdgeInsets& content_edge_insets() const { return m_content_edge_insets; }

    inline const LG::Font& font() const { return m_font; }
    void set_font(const LG::Font& font) { m_font = font, recalc_bounds(); }

    virtual void display(const LG::Rect& rect) override;
    virtual void hover_begin(const LG::Point<int>& location) override;
    virtual void hover_end() override;

    virtual void click_began(const LG::Point<int>& location) override;
    virtual void click_ended() override;

    void set_type(Type type) { m_button_type = type; }

protected:
    Button(View* superview, const LG::Rect& frame);

private:
    void recalc_bounds();
    size_t text_height() const;
    size_t text_width();

    static constexpr uint32_t system_background_color() { return 0x00EBEBEB; }
    Type m_button_type { Type::System };

    std::string m_title {};
    LG::Color m_title_color { LG::Color::White };
    LG::Font m_font { LG::Font::system_font() };

    EdgeInsets m_content_edge_insets { 12, 12, 12, 12 };
};

} // namespace UI