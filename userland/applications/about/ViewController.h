/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "AboutLineView.h"
#include <libui/App.h>
#include <libui/Button.h>
#include <libui/Label.h>
#include <libui/View.h>
#include <libui/ViewController.h>
#include <libui/Window.h>
#include <memory>
#include <sys/types.h>
#include <sys/utsname.h>

class ViewController : public UI::ViewController<UI::View> {
public:
    ViewController(UI::View& view)
        : UI::ViewController<UI::View>(view)
    {
    }
    virtual ~ViewController() = default;

    void view_did_load() override
    {
        view().set_background_color(LG::Color::LightSystemBackground);

        utsname_t uts;
        int rc = uname(&uts);

        auto& label = view().add_subview<UI::Label>(LG::Rect(0, 0, 16, 24));
        label.set_text_color(LG::Color::DarkSystemText);
        label.set_text("About");
        label.set_font(LG::Font::system_font(LG::Font::SystemTitleSize));
        label.set_width(label.preferred_width());

        auto& name_label = view().add_subview<AboutLineView>(LG::Rect(0, 0, 16, 16), "Name:", uts.sysname);
        auto& cpu_label = view().add_subview<AboutLineView>(LG::Rect(0, 0, 16, 16), "CPU:", uts.machine);
        auto& version_label = view().add_subview<AboutLineView>(LG::Rect(0, 0, 16, 16), "Version:", uts.release);

        auto& button = view().add_subview<UI::Button>(LG::Rect(0, 0, 10, 10));
        button.set_background_color(LG::Color::LightSystemButton);
        button.set_title("System info");
        button.set_title_color(LG::Color::DarkSystemText);

        auto& footer = view().add_subview<UI::Label>(LG::Rect(0, 0, 16, 16));
        footer.set_text_color(LG::Color::DarkSystemText);
        footer.set_text("(c) 2020-2022");
        footer.set_width(footer.preferred_width());

        view().add_constraint(UI::Constraint(label, UI::Constraint::Attribute::Left, UI::Constraint::Relation::Equal, UI::SafeArea::Left));
        view().add_constraint(UI::Constraint(label, UI::Constraint::Attribute::Top, UI::Constraint::Relation::Equal, UI::SafeArea::Top));

        view().add_constraint(UI::Constraint(name_label, UI::Constraint::Attribute::Left, UI::Constraint::Relation::Equal, UI::SafeArea::Left));
        view().add_constraint(UI::Constraint(name_label, UI::Constraint::Attribute::Top, UI::Constraint::Relation::Equal, label, UI::Constraint::Attribute::Bottom, 1, UI::Padding::AfterTitle));

        view().add_constraint(UI::Constraint(cpu_label, UI::Constraint::Attribute::Left, UI::Constraint::Relation::Equal, UI::SafeArea::Left));
        view().add_constraint(UI::Constraint(cpu_label, UI::Constraint::Attribute::Top, UI::Constraint::Relation::Equal, name_label, UI::Constraint::Attribute::Bottom, 1, 4));

        view().add_constraint(UI::Constraint(version_label, UI::Constraint::Attribute::Left, UI::Constraint::Relation::Equal, UI::SafeArea::Left));
        view().add_constraint(UI::Constraint(version_label, UI::Constraint::Attribute::Top, UI::Constraint::Relation::Equal, cpu_label, UI::Constraint::Attribute::Bottom, 1, 4));

        view().add_constraint(UI::Constraint(button, UI::Constraint::Attribute::Left, UI::Constraint::Relation::Equal, UI::SafeArea::Left));
        view().add_constraint(UI::Constraint(button, UI::Constraint::Attribute::Top, UI::Constraint::Relation::Equal, version_label, UI::Constraint::Attribute::Bottom, 1, 8));

        view().add_constraint(UI::Constraint(footer, UI::Constraint::Attribute::CenterX, UI::Constraint::Relation::Equal, view(), UI::Constraint::Attribute::CenterX, 1, 0));
        view().add_constraint(UI::Constraint(footer, UI::Constraint::Attribute::Bottom, UI::Constraint::Relation::Equal, view(), UI::Constraint::Attribute::Bottom, 1, -UI::SafeArea::Bottom));

        view().set_needs_layout();
    }

private:
};