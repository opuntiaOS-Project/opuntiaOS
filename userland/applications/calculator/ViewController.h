/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libui/App.h>
#include <libui/Button.h>
#include <libui/Label.h>
#include <libui/StackView.h>
#include <libui/View.h>
#include <libui/ViewController.h>
#include <libui/Window.h>
#include <memory>
#include <sys/types.h>
#include <sys/utsname.h>

// TODO: Add enum for colors.

class ViewController : public UI::ViewController<UI::View> {
public:
    ViewController(UI::View& view)
        : UI::ViewController<UI::View>(view)
    {
    }
    virtual ~ViewController() = default;

    static constexpr const char* button_titles[20] = {
        "c",
        "+/-",
        "%",
        "/",
        "7",
        "8",
        "9",
        "*",
        "4",
        "5",
        "6",
        "-",
        "1",
        "2",
        "3",
        "+",
        "0",
        "00",
        ",",
        "=",
    };

    constexpr size_t button_size() { return 48; }
    constexpr size_t spacing_size() { return 8; }

    bool is_operation(char a) { return a == '*' || a == '/' || a == '+' || a == '-'; }

    void on_button_click(UI::Button* sender)
    {
        // Check if it is a number
        if (sender->title().size() == 1 && '0' <= sender->title()[0] && sender->title()[0] <= '9') {
            m_current_number.push_back(sender->title()[0]);
            m_answer_label_ptr->set_text(m_current_number);
            return;
        }

        // Check if it is an operation
        if (sender->title().size() == 1 && is_operation(sender->title()[0])) {
            var_number1 = std::atoi(m_current_number.c_str());
            m_current_number.clear();
            m_operation = sender->title()[0];
            m_active_operation_button = sender;
            sender->set_background_color(LG::Color(165, 201, 242));
            return;
        }

        if (sender->title().size() == 1 && sender->title()[0] == '=') {
            int var_number2 = std::atoi(m_current_number.c_str());
            switch (m_operation) {
            case '+':
                var_number1 = var_number1 + var_number2;
                m_current_number = std::to_string(var_number1);
                m_answer_label_ptr->set_text(m_current_number);
                m_active_operation_button->set_background_color(LG::Color(231, 240, 250));
                break;
            case '-':
                var_number1 = var_number1 - var_number2;
                m_current_number = std::to_string(var_number1);
                m_answer_label_ptr->set_text(m_current_number);
                m_active_operation_button->set_background_color(LG::Color(231, 240, 250));
                break;
            case '*':
                var_number1 = var_number1 * var_number2;
                m_current_number = std::to_string(var_number1);
                m_answer_label_ptr->set_text(m_current_number);
                m_active_operation_button->set_background_color(LG::Color(231, 240, 250));
                break;
            case '/':
                m_active_operation_button->set_background_color(LG::Color(231, 240, 250));
                if (var_number2 == 0) {
                    m_current_number = "";
                    m_answer_label_ptr->set_text("Error");
                    return;
                }
                var_number1 = var_number1 / var_number2;
                m_current_number = std::to_string(var_number1);
                m_answer_label_ptr->set_text(m_current_number);
                break;
            default:
                m_current_number = "";
                m_answer_label_ptr->set_text("Error");
            }
            return;
        }

        if (sender->title().size() == 1 && sender->title()[0] == 'c') {
            m_operation = ' ';
            m_active_operation_button = nullptr;
            m_answer_label_ptr->set_text("0");
            m_current_number.clear();
            return;
        }
    }

    void view_did_load() override
    {
        view().set_background_color(LG::Color::LightSystemBackground);

        auto& main_stackview = view().add_subview<UI::StackView>(LG::Rect(0, 0, 0, button_size() * 5 + spacing_size() * 4));
        main_stackview.set_background_color(LG::Color::LightSystemBackground);
        main_stackview.set_axis(UI::LayoutConstraints::Axis::Vertical);
        main_stackview.set_spacing(spacing_size());
        view().add_constraint(UI::Constraint(main_stackview, UI::Constraint::Attribute::Left, UI::Constraint::Relation::Equal, UI::SafeArea::Left));
        view().add_constraint(UI::Constraint(main_stackview, UI::Constraint::Attribute::Right, UI::Constraint::Relation::Equal, UI::SafeArea::Right));
        view().add_constraint(UI::Constraint(main_stackview, UI::Constraint::Attribute::Bottom, UI::Constraint::Relation::Equal, UI::SafeArea::Bottom));

        for (int i = 0; i < 5; i++) {
            auto& stackview = main_stackview.add_arranged_subview<UI::StackView>();
            stackview.set_background_color(LG::Color::LightSystemBackground);
            stackview.set_distribution(UI::StackView::Distribution::EqualSpacing);
            main_stackview.add_constraint(UI::Constraint(stackview, UI::Constraint::Attribute::Left, UI::Constraint::Relation::Equal, main_stackview, UI::Constraint::Attribute::Left, 1, 0));
            main_stackview.add_constraint(UI::Constraint(stackview, UI::Constraint::Attribute::Right, UI::Constraint::Relation::Equal, main_stackview, UI::Constraint::Attribute::Right, 1, 0));
            main_stackview.add_constraint(UI::Constraint(stackview, UI::Constraint::Attribute::Height, UI::Constraint::Relation::Equal, button_size()));

            for (int j = 0; j < 4; j++) {
                auto& view1 = stackview.add_arranged_subview<UI::Button>();
                if (j == 3 && i == 4) {
                    view1.set_background_color(LG::Color(198, 166, 242));
                    view1.set_title_color(LG::Color::White);
                } else if (j == 3 || i == 0) {
                    view1.set_background_color(LG::Color(231, 240, 250));
                    view1.set_title_color(LG::Color(35, 70, 106));
                } else {
                    view1.set_background_color(LG::Color::LightSystemButton);
                    view1.set_title_color(LG::Color::DarkSystemText);
                }
                view1.set_font(LG::Font::system_bold_font(14));
                view1.set_title(button_titles[i * 4 + j]);
                view1.set_alignment(UI::Text::Alignment::Center);
                view1.add_target([this](UI::View* sender) { this->on_button_click(static_cast<UI::Button*>(sender)); }, UI::Event::Type::MouseUpEvent);
                view1.layer().set_corner_mask(LG::CornerMask(button_size() / 2));
                stackview.add_constraint(UI::Constraint(view1, UI::Constraint::Attribute::Height, UI::Constraint::Relation::Equal, button_size()));
                stackview.add_constraint(UI::Constraint(view1, UI::Constraint::Attribute::Width, UI::Constraint::Relation::Equal, button_size()));
            }
        }

        auto& answer_label = view().add_subview<UI::Label>(LG::Rect(0, 0, 0, 40));
        answer_label.set_text("0");
        answer_label.set_text_color(LG::Color::DarkSystemText);
        answer_label.set_alignment(UI::Text::Alignment::Right);
        answer_label.set_font(LG::Font::system_font(36));
        m_answer_label_ptr = &answer_label;
        view().add_constraint(UI::Constraint(answer_label, UI::Constraint::Attribute::Left, UI::Constraint::Relation::Equal, UI::SafeArea::Left));
        view().add_constraint(UI::Constraint(answer_label, UI::Constraint::Attribute::Right, UI::Constraint::Relation::Equal, UI::SafeArea::Right));
        view().add_constraint(UI::Constraint(answer_label, UI::Constraint::Attribute::Bottom, UI::Constraint::Relation::Equal, main_stackview, UI::Constraint::Attribute::Top, 1, -8));

        view().set_needs_layout();
    }

private:
    UI::Label* m_answer_label_ptr { nullptr };
    std::string m_current_number;
    int var_number1 { 0 };
    char m_operation;
    UI::Button* m_active_operation_button { nullptr };
};