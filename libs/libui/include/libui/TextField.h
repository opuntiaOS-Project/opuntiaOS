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

class TextField : public Control {
    UI_OBJECT();

public:
    enum Type {
        System,
        Custom,
    };

    ~TextField() = default;

    const std::string& text() const { return m_text; }
    void set_text(const std::string& text) { m_text = text, set_needs_display(); }
    void set_text(std::string&& text) { m_text = std::move(text), set_needs_display(); }

    void set_placeholder_text(const std::string& text) { m_placeholder_text = text, set_needs_display(); }
    void set_placeholder_text(std::string&& text) { m_placeholder_text = std::move(text), set_needs_display(); }

    void set_text_color(const LG::Color& color) { m_text_color = color; }
    const LG::Color& text_color() const { return m_text_color; }

    void set_content_edge_insets(const EdgeInsets& ei) { m_content_edge_insets = ei; }
    const EdgeInsets& content_edge_insets() const { return m_content_edge_insets; }

    void set_font(const LG::Font& font) { m_font = font, set_needs_display(); }
    inline const LG::Font& font() const { return m_font; }

    virtual void display(const LG::Rect& rect) override;
    virtual void mouse_entered(const LG::Point<int>& location) override;
    virtual void mouse_exited() override;

    virtual void mouse_down(const LG::Point<int>& location) override;
    virtual void mouse_up() override;

protected:
    TextField(View* superview, const LG::Rect& frame);

private:
    size_t text_width(const std::string& text);
    size_t text_height(const std::string& text) const;

    std::string m_text {};
    std::string m_placeholder_text {};
    LG::Color m_text_color { LG::Color::DarkSystemText };
    LG::Font m_font { LG::Font::system_font() };

    Text::Alignment m_alignment { Text::Alignment::Left };
    EdgeInsets m_content_edge_insets { 12, 12, 12, 12 };
};

} // namespace UI