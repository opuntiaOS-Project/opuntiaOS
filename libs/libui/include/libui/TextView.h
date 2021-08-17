/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libg/Size.h>
#include <libui/Constants/Layout.h>
#include <libui/EdgeInsets.h>
#include <libui/ScrollView.h>
#include <string>
#include <utility>

namespace UI {

class TextView : public ScrollView {
    UI_OBJECT();

public:
    ~TextView() = default;

    void set_text(const std::string& text) { m_text = text, recalc_text_size(), set_needs_display(); }
    void set_text(std::string&& text) { m_text = std::move(text), recalc_text_size(), set_needs_display(); }
    const std::string& text() const { return m_text; }

    void set_text_color(const LG::Color& color) { m_text_color = color, set_needs_display(); }
    const LG::Color& text_color() const { return m_text_color; }

    void set_font(const LG::Font& font) { m_font = font, recalc_text_size(), set_needs_display(); }
    inline const LG::Font& font() const { return m_font; }

    virtual void display(const LG::Rect& rect) override;
    virtual void mouse_entered(const LG::Point<int>& location) override;
    virtual void mouse_exited() override;

protected:
    TextView(View* superview, const LG::Rect&);
    TextView(View* superview, Window* window, const LG::Rect& frame);

private:
    void recalc_text_size();

    std::string m_text {};
    LG::Color m_text_color { LG::Color::Black };
    LG::Font m_font { LG::Font::system_font() };
};

} // namespace UI