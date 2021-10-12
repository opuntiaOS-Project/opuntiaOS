/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libg/Font.h>
#include <libui/Label.h>
#include <libui/View.h>
#include <string>

class AboutLineView : public UI::View {
    UI_OBJECT();

public:
    AboutLineView(UI::View* superview, const LG::Rect& frame, std::string title, std::string content)
        : UI::View(superview, frame)
        , m_title(title)
        , m_content(content)
    {
        const int spacing = 4;
        set_background_color(LG::Color::LightSystemBackground);

        auto& label = add_subview<UI::Label>(LG::Rect(0, 0, 16, 16));
        label.set_text_color(LG::Color::DarkSystemText);
        label.set_text(m_title);
        label.set_width(label.preferred_width());

        auto& target_label = add_subview<UI::Label>(LG::Rect(0, 0, 16, 16));
        target_label.set_text_color(LG::Color::DarkSystemText);
        target_label.set_text(m_content);
        target_label.set_width(target_label.preferred_width());

        add_constraint(UI::Constraint(target_label, UI::Constraint::Attribute::Left, UI::Constraint::Relation::Equal, label, UI::Constraint::Attribute::Right, 1, spacing));
        add_constraint(UI::Constraint(target_label, UI::Constraint::Attribute::Top, UI::Constraint::Relation::Equal, label, UI::Constraint::Attribute::Top, 1, 0));

        set_width(label.preferred_width() + spacing + target_label.preferred_width());
    }

    AboutLineView(UI::View* superview, UI::Window* window, const LG::Rect& frame, std::string title, std::string content)
        : UI::View(superview, window, frame)
        , m_title(title)
        , m_content(content)
    {
        const int spacing = 4;
        set_background_color(LG::Color::LightSystemBackground);

        auto& label = add_subview<UI::Label>(LG::Rect(0, 0, 16, 16));
        label.set_text_color(LG::Color::DarkSystemText);
        label.set_text(m_title);
        label.set_width(label.preferred_width());

        auto& target_label = add_subview<UI::Label>(LG::Rect(0, 0, 16, 16));
        target_label.set_text_color(LG::Color::DarkSystemText);
        target_label.set_text(m_content);
        target_label.set_width(target_label.preferred_width());

        add_constraint(UI::Constraint(target_label, UI::Constraint::Attribute::Left, UI::Constraint::Relation::Equal, label, UI::Constraint::Attribute::Right, 1, spacing));
        add_constraint(UI::Constraint(target_label, UI::Constraint::Attribute::Top, UI::Constraint::Relation::Equal, label, UI::Constraint::Attribute::Top, 1, 0));

        set_width(label.preferred_width() + spacing + target_label.preferred_width());
    }

    ~AboutLineView() = default;

private:
    std::string m_title {};
    std::string m_content {};
};