/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libfoundation/EventLoop.h>
#include <libg/Color.h>
#include <libui/Context.h>
#include <libui/StackView.h>

namespace UI {

StackView::StackView(View* superview, const LG::Rect& frame)
    : View(superview, frame)
{
}

StackView::StackView(View* superview, Window* window, const LG::Rect& frame)
    : View(superview, window, frame)
{
}

bool StackView::receive_layout_event(const LayoutEvent& event, bool force_layout_if_not_target)
{
    // StackView uses receive_layout_event to recalculate positions of
    // subviews.
    bool res = View::receive_layout_event(event, force_layout_if_not_target);
    if (this == event.target() || force_layout_if_not_target) {
        recalc_subviews_positions();
    }
    return res;
}

size_t StackView::recalc_subview_min_x(View* view)
{
    size_t width = bounds().width();
    switch (alignment()) {
    case Alignment::Leading:
        return 0;
    case Alignment::Center:
        return (width - view->bounds().width()) / 2;
    case Alignment::Trailing:
        return width - view->bounds().width();
    default:
        break;
    }
    return 0;
}

size_t StackView::recalc_subview_min_y(View* view)
{
    size_t height = bounds().height();
    switch (alignment()) {
    case Alignment::Leading:
        return 0;
    case Alignment::Center:
        return (height - view->bounds().height()) / 2;
    case Alignment::Trailing:
        return height - view->bounds().height();
    default:
        break;
    }
    return 0;
}

void StackView::recalc_fill_equally()
{
    // TODO: May be to reinterpret contstraints here?
    size_t total_spacing = spacing() * (m_views.size() - 1);
    if (axis() == LayoutConstraints::Axis::Horizontal) {
        size_t width = (bounds().width() - total_spacing) / m_views.size();
        for (int i = 0; i < m_views.size(); i++) {
            constraint_interpreter(Constraint(*m_views[i], Constraint::Attribute::Width, Constraint::Relation::Equal, width));
        }
    } else {
        size_t height = (bounds().height() - total_spacing) / m_views.size();
        for (int i = 0; i < m_views.size(); i++) {
            constraint_interpreter(Constraint(*m_views[i], Constraint::Attribute::Height, Constraint::Relation::Equal, height));
        }
    }
}

size_t StackView::recalc_total_content_width()
{
    size_t res = 0;
    for (int i = 0; i < m_views.size(); i++) {
        res += m_views[i]->frame().width();
    }
    return res;
}

size_t StackView::recalc_total_content_height()
{
    size_t res = 0;
    for (int i = 0; i < m_views.size(); i++) {
        res += m_views[i]->frame().width();
    }
    return res;
}

size_t StackView::recalc_spacing()
{
    switch (distribution()) {
    case Distribution::Standard:
        return spacing();
    case Distribution::EqualSpacing:
        if (axis() == LayoutConstraints::Axis::Horizontal) {
            return recalc_equal_spacing_horizontal();
        } else {
            return recalc_equal_spacing_vertical();
        }
    case Distribution::FillEqually:
        recalc_fill_equally();
        return spacing();
    default:
        break;
    }
    return 0;
}

// recalc_subviews_positions recalculates the posistion of all subviews.
// You have to call set_needs_layout instread of direct call to recalc_subviews_positions.
void StackView::recalc_subviews_positions()
{
    size_t spacing = recalc_spacing();
    size_t primary_coord = 0;
    if (axis() == LayoutConstraints::Axis::Horizontal) {
        for (int i = 0; i < m_views.size(); i++) {
            m_views[i]->frame().set_y(recalc_subview_min_y(m_views[i]));
            m_views[i]->frame().set_x(primary_coord);
            primary_coord += m_views[i]->frame().width() + spacing;
        }
    } else {
        for (int i = 0; i < m_views.size(); i++) {
            m_views[i]->frame().set_x(recalc_subview_min_x(m_views[i]));
            m_views[i]->frame().set_y(primary_coord);
            primary_coord += m_views[i]->frame().height() + spacing;
        }
    }
}

} // namespace UI