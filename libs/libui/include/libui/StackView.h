/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libg/Font.h>
#include <libui/Constants/Layout.h>
#include <libui/EdgeInsets.h>
#include <libui/View.h>
#include <string>
#include <utility>

namespace UI {

class StackView : public View {
    UI_OBJECT();

public:
    enum class Distribution {
        Standard,
        FillEqually,
        EqualSpacing,
    };

    enum class Alignment {
        Leading,
        Center,
        Trailing,
    };

    ~StackView() = default;

    template <class T, class... Args>
    T& add_arranged_subview(Args&&... args)
    {
        T& view = add_subview<T>(LG::Rect(0, 0, 0, 0), std::forward<Args>(args)...);
        m_views.push_back(&view);
        return view;
    }

    const std::vector<View*>& arranged_subviews() const { return m_views; }
    std::vector<View*>& arranged_subviews() { return m_views; }

    void set_axis(LayoutConstraints::Axis axis) { m_axis = axis; }
    LayoutConstraints::Axis axis() const { return m_axis; }

    void set_distribution(Distribution dist) { m_distribution = dist; }
    Distribution distribution() const { return m_distribution; }

    void set_alignment(Alignment alignment) { m_alignment = alignment; }
    Alignment alignment() const { return m_alignment; }

    void set_spacing(size_t spacing) { m_spacing = spacing; }
    size_t spacing() const { return m_spacing; }

    virtual bool receive_layout_event(const LayoutEvent&, bool force_layout_if_not_target = false) override;

protected:
    StackView(View* superview, const LG::Rect&);
    StackView(View* superview, Window* window, const LG::Rect& frame);

private:
    void recalc_subviews_positions();
    void recalc_fill_equally();
    size_t recalc_total_content_width();
    size_t recalc_total_content_height();
    size_t recalc_subview_min_x(View*);
    size_t recalc_subview_min_y(View*);
    size_t recalc_spacing();
    size_t recalc_equal_spacing_horizontal() { return m_views.size() > 2 ? (bounds().width() - recalc_total_content_width()) / (m_views.size() - 1) : 0; }
    size_t recalc_equal_spacing_vertical() { return m_views.size() > 2 ? (bounds().height() - recalc_total_content_height()) / (m_views.size() - 1) : 0; }

    std::vector<View*> m_views;
    size_t m_spacing { 0 };
    Alignment m_alignment { Alignment::Leading };
    Distribution m_distribution { Distribution::Standard };
    LayoutConstraints::Axis m_axis { LayoutConstraints::Axis::Horizontal };
};

} // namespace UI