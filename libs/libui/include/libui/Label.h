/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libg/Font.h>
#include <libui/EdgeInsets.h>
#include <libui/View.h>
#include <std/String.h>

namespace UI {

class Label : public View {
public:
    explicit Label(const LG::Rect&);

    const String& text() const { return m_text; }
    void set_text(const String& text) { m_text = text; }
    void set_text(String&& text) { m_text = std::move(text); }

    void set_text_color(const LG::Color& color) { m_text_color = color; }
    const LG::Color& text_color() const { return m_text_color; }

    void set_content_edge_insets(const EdgeInsect& ei) { m_content_edge_insets = ei; }
    const EdgeInsect& content_edge_insets() const { return m_content_edge_insets; }

    inline const LG::Font& font() const { return m_font; }
    void set_font(const LG::Font& font) { m_font = font, recalc_bounds(); }

    inline size_t preferred_width() const { return text_width() + m_content_edge_insets.left() + m_content_edge_insets.right(); }

    virtual void display(const LG::Rect& rect) override;
    virtual void hover_begin(const LG::Point<int>& location) override;
    virtual void hover_end() override;

private:
    void recalc_bounds();
    size_t text_height() const;
    size_t text_width() const;

    String m_text {};
    LG::Color m_text_color { LG::Color::Black };
    LG::Font m_font { LG::Font::system_font() };

    EdgeInsect m_content_edge_insets {};
};

}