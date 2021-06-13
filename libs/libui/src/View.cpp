/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libfoundation/EventLoop.h>
#include <libg/Color.h>
#include <libui/Context.h>
#include <libui/View.h>

namespace UI {

View::View(View* superview, const LG::Rect& frame)
    : m_frame(frame)
    , m_superview(superview)
    , m_window(superview ? superview->window() : nullptr)
    , m_bounds(0, 0, frame.width(), frame.height())
{
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

void View::layout_subviews()
{
    // TODO: Apply topsort to find the right order.
    for (const auto& constraint : m_constrints) {
        constraint_interpreter(constraint);
    }
}

void View::set_needs_display(const LG::Rect& rect)
{
    auto display_rect = rect;
    display_rect.intersect(bounds());
    if (has_superview()) {
        display_rect.offset_by(frame().origin());
        superview()->set_needs_display(display_rect);
    } else {
        send_display_message_to_self(*window(), display_rect);
    }
}

void View::display(const LG::Rect& rect)
{
    Context ctx(*this);
    ctx.set_fill_color(background_color());
    ctx.fill(rect);
}

void View::did_display(const LG::Rect& rect)
{
}

void View::mouse_moved(const LG::Point<int>& location)
{
}

void View::hover_begin(const LG::Point<int>& location)
{
    m_hovered = true;
    set_needs_display();
}

void View::hover_end()
{
    m_hovered = false;
    m_active = false;
    set_needs_display();
}

void View::click_began(const LG::Point<int>& location)
{
    m_active = true;
    set_needs_display();
}

void View::click_ended()
{
    m_active = false;
    set_needs_display();
}

void View::receive_mouse_move_event(MouseEvent& event)
{
    auto location = LG::Point<int>(event.x(), event.y());
    if (!is_hovered()) {
        hover_begin(location);
    }

    foreach_subview([&](View& subview) -> bool {
        if (subview.is_hovered() && !subview.frame().contains(event.x(), event.y())) {
            LG::Point<int> point(event.x(), event.y());
            point.offset_by(-subview.frame().origin());
            MouseLeaveEvent mle(point.x(), point.y());
            subview.receive_mouse_leave_event(mle);
        } else if (!subview.is_hovered() && subview.frame().contains(event.x(), event.y())) {
            LG::Point<int> point(event.x(), event.y());
            point.offset_by(-subview.frame().origin());
            MouseEvent me(point.x(), point.y());
            subview.receive_mouse_move_event(me);
        }
        return true;
    });

    mouse_moved(location);
    Responder::receive_mouse_move_event(event);
}

void View::receive_mouse_action_event(MouseActionEvent& event)
{
    if (event.type() == MouseActionType::ClickBegan) {
        click_began(LG::Point<int>(event.x(), event.y()));
    } else if (event.type() == MouseActionType::ClickEnded) {
        click_ended();
    }

    Responder::receive_mouse_action_event(event);
}

void View::receive_mouse_leave_event(MouseLeaveEvent& event)
{
    if (!is_hovered()) {
        return;
    }

    foreach_subview([&](View& subview) -> bool {
        if (subview.is_hovered()) {
            LG::Point<int> point(event.x(), event.y());
            point.offset_by(-subview.frame().origin());
            MouseLeaveEvent mle(point.x(), point.y());
            subview.receive_mouse_leave_event(mle);
        }
        return true;
    });

    hover_end();
    Responder::receive_mouse_leave_event(event);
}

void View::receive_keyup_event(KeyUpEvent&)
{
}

void View::receive_keydown_event(KeyDownEvent&)
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

    if (!has_superview()) {
        // Only superview send invalidate_message to server.
        bool success = send_invalidate_message_to_server(event.bounds());
    }

    Responder::receive_display_event(event);
}

bool View::receive_layout_event(const LayoutEvent& event, bool force_layout_if_not_target)
{
    bool need_to_layout = (this == event.target()) | force_layout_if_not_target;
    if (need_to_layout) {
        layout_subviews();
    }

    foreach_subview([&](View& subview) -> bool {
        bool found_target = subview.receive_layout_event(event, need_to_layout);
        return force_layout_if_not_target || !found_target;
    });

    return need_to_layout;
}

} // namespace UI