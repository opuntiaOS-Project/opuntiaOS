/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
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
        CenterX,
        CenterY,
        Width,
        Height,
    };

    enum class Relation {
        Equal,
        // LessThanOrEqual,
        // GreaterThanOrEqual,
    };

    Constraint(View& item, Constraint::Attribute attr, Constraint::Relation related_by, View& rel_item, Constraint::Attribute to_attr, int multiplier, int constant)
        : m_item(&item)
        , m_attr(attr)
        , m_related_by(related_by)
        , m_rel_item(&rel_item)
        , m_to_attr(to_attr)
        , m_multiplier(multiplier)
        , m_constant(constant)
    {
    }

    Constraint(View& item, Constraint::Attribute attr, Constraint::Relation related_by, int constant)
        : m_item(&item)
        , m_attr(attr)
        , m_related_by(related_by)
        , m_rel_item(nullptr)
        , m_to_attr(Constraint::Attribute::NotAnAttr)
        , m_multiplier(1)
        , m_constant(constant)
    {
    }

    ~Constraint() = default;

    UI::View* item() const { return m_item; }
    Constraint::Attribute attribute() const { return m_attr; }
    Constraint::Relation relation() const { return m_related_by; }
    UI::View* rel_item() const { return m_rel_item; }
    Constraint::Attribute rel_attribute() const { return m_to_attr; }
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

        case UI::Constraint::Attribute::CenterX:
            return (T)rect.mid_x();

        case UI::Constraint::Attribute::CenterY:
            return (T)rect.mid_y();

        case UI::Constraint::Attribute::Width:
            return (T)rect.width();

        case UI::Constraint::Attribute::Height:
            return (T)rect.height();

        default:
            return (T)0;
        }
    }

    template <UI::Constraint::Attribute attr, typename T>
    static constexpr inline void set_attribute(LG::Rect& rect, T m_value)
    {
        if constexpr (attr == UI::Constraint::Attribute::Top) {
            rect.set_y(m_value);
            return;
        } else if constexpr (attr == UI::Constraint::Attribute::Bottom) {
            rect.set_y(m_value - rect.height());
            return;
        } else if constexpr (attr == UI::Constraint::Attribute::Left) {
            rect.set_x(m_value);
            return;
        } else if constexpr (attr == UI::Constraint::Attribute::Right) {
            rect.set_x(m_value - rect.width());
            return;
        } else if constexpr (attr == UI::Constraint::Attribute::CenterX) {
            rect.set_x(m_value - (rect.width() / 2));
            return;
        } else if constexpr (attr == UI::Constraint::Attribute::CenterY) {
            rect.set_y(m_value - (rect.height() / 2));
            return;
        } else if constexpr (attr == UI::Constraint::Attribute::Width) {
            rect.set_width(m_value);
            return;
        } else if constexpr (attr == UI::Constraint::Attribute::Height) {
            rect.set_height(m_value);
            return;
        }
    }

private:
    UI::View* m_item;
    Constraint::Attribute m_attr;
    Constraint::Relation m_related_by;
    UI::View* m_rel_item;
    Constraint::Attribute m_to_attr;
    int m_multiplier { 0 };
    int m_constant { 0 };
};

} // namespace UI