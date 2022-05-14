/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libg/Font.h>
#include <libui/Constants/Text.h>
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

    void set_content_edge_insets(const EdgeInsets& ei) { m_content_edge_insets = ei, recalc_bounds(); }
    const EdgeInsets& content_edge_insets() const { return m_content_edge_insets; }

    void set_font(const LG::Font& font) { m_font = font, recalc_bounds(); }
    inline const LG::Font& font() const { return m_font; }

    void set_alignment(Text::Alignment alignment) { m_alignment = alignment; }
    Text::Alignment alignment() const { return m_alignment; }

    virtual void display(const LG::Rect& rect) override;
    virtual void mouse_entered(const LG::Point<int>& location) override;
    virtual void mouse_exited() override;

    virtual void mouse_down(const LG::Point<int>& location) override;
    virtual void mouse_up() override;

    void set_type(Type type) { m_button_type = type; }

protected:
    Button(View* superview, const LG::Rect& frame);

private:
    void recalc_bounds();
    size_t text_height() const;
    size_t text_width();

    Type m_button_type { Type::System };

    std::string m_title {};
    LG::Color m_title_color { LG::Color::White };
    LG::Font m_font { LG::Font::system_font() };

    Text::Alignment m_alignment { Text::Alignment::Left };
    EdgeInsets m_content_edge_insets { 12, 12, 12, 12 };
};

} // namespace UI