/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "View.h"
#include "Context.h"
#include <libg/Color.h>
#include <syscalls.h>

namespace UI {

View::View(const LG::Rect& frame)
    : m_frame(frame)
    , m_bounds(0, 0, frame.width(), frame.height())
{
}

void View::add_subview(View* subview)
{
    subview->set_window(window());
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

LG::Rect View::frame_in_window()
{
    View* view = this;
    auto rect = frame();
    while (view->has_superview()) {
        view = view->superview();
        rect.offset_by(view->frame().origin());
    }
    return rect;
}

void View::set_needs_display(const LG::Rect& rect)
{
    auto display_rect = rect;
    display_rect.intersect(bounds());
    if (has_superview()) {
        display_rect.offset_by(frame().origin());
        superview()->set_needs_display(display_rect);
    } else {
        bool success = send_invalidate_message(display_rect);
    }
}

void View::display(const LG::Rect& rect)
{
    Context ctx(*this);
    if (has_superview()) {
        ctx.set_fill_color(LG::Color::Red);
    } else {
        ctx.set_fill_color(LG::Color::White);
    }

    ctx.fill(rect);
}

void View::did_display(const LG::Rect& rect)
{
}

void View::receive_mouse_event(MouseEvent&)
{
}

void View::receive_display_event(DisplayEvent& event)
{
    event.bounds().intersect(bounds());
    display(event.bounds());
    foreach_subview([&](View& subview) -> bool {
        auto bounds = event.bounds();
        if (bounds.intersects(subview.frame())) {
            bounds.offset_by(-subview.frame().origin());
            DisplayEvent own_event(bounds);
            subview.receive_display_event(own_event);
        }
        return true;
    });
    did_display(event.bounds());
}

}