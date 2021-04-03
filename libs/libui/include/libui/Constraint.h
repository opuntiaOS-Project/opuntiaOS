/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

namespace UI {
class View;

class Constraint {
public:
    enum class Attribute {
        NotAnAttr,
        Left,
        Right,
        Top,
        Bottom,
    };

    enum class Relation {
        Equal,
        // LessThanOrEqual,
        // GreaterThanOrEqual,
    };

    Constraint(View& item, Constraint::Attribute attr, Constraint::Relation related_by, View& to_item, Constraint::Attribute to_attr, int multiplier, int constant)
        : m_item(&item)
        , m_attr(attr)
        , m_related_by(related_by)
        , m_to_item(&to_item)
        , m_to_attr(to_attr)
        , m_multiplier(multiplier)
        , m_constant(constant)
    {
    }

    Constraint(View& item, Constraint::Attribute attr, Constraint::Relation related_by, int constant)
        : m_item(&item)
        , m_attr(attr)
        , m_related_by(related_by)
        , m_to_item(nullptr)
        , m_to_attr(Constraint::Attribute::NotAnAttr)
        , m_multiplier(1)
        , m_constant(constant)
    {
    }

    ~Constraint() = default;

    UI::View* item() const { return m_item; }
    Constraint::Attribute attribute() const { return m_attr; }
    Constraint::Relation relation() const { return m_related_by; }
    UI::View* to_item() const { return m_to_item; }
    Constraint::Attribute to_attribute() const { return m_to_attr; }
    int multiplier() const { return m_multiplier; }
    int constant() const { return m_constant; }

    template <typename T>
    static inline T get_attribute(const LG::Rect& rect, UI::Constraint::Attribute attr)
    {
        switch (attr) {

        case UI::Constraint::Attribute::Top:
            return (T)rect.min_y();

        case UI::Constraint::Attribute::Bottom:
            return (T)rect.max_y();

        case UI::Constraint::Attribute::Left:
            return (T)rect.min_x();

        case UI::Constraint::Attribute::Right:
            return (T)rect.max_x();

        default:
            return (T)0;
        }
    }

    template <typename T>
    inline void set_attribute(const LG::Rect& rect, UI::Constraint::Attribute attr, T m_value)
    {
        switch (attr) {
        case UI::Constraint::Attribute::Top:
            rect.set_y(m_value);
            return;

        case UI::Constraint::Attribute::Bottom:
            rect.set_y(m_value - rect.height());
            return;

        case UI::Constraint::Attribute::Left:
            rect.set_x(m_value);
            return;

        case UI::Constraint::Attribute::Right:
            rect.set_x(m_value - rect.width());
            return;

        default:
            return;
        }
    }

private:
    UI::View* m_item;
    Constraint::Attribute m_attr;
    Constraint::Relation m_related_by;
    UI::View* m_to_item;
    Constraint::Attribute m_to_attr;
    int m_multiplier { 0 };
    int m_constant { 0 };
};

} // namespace UI