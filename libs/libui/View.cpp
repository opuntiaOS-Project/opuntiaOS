/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "View.h"

namespace UI {

View::View(const LG::Rect& frame)
    : m_frame(frame)
    , m_bounds(0, 0, frame.width(), frame.height())
{
}

void View::add_subview(View* subview)
{
    subview->set_superview(this);
    m_subviews.push_back(subview);
}

void View::remove_from_superview()
{
}

View* View::hit_test(const LG::Point<int>& point)
{
    if (auto* subview = subview_at(point)) {
        return subview->hit_test(point - subview->frame().origin());
    }
    return this;
}

void View::receive_mouse_event(UniquePtr<MouseEvent>)
{
}

}