/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libfoundation/Math.h>
#include <libg/Color.h>
#include <libui/Context.h>
#include <libui/ScrollView.h>
#include <utility>

namespace UI {

ScrollView::ScrollView(View* superview, const LG::Rect& frame)
    : View(superview, frame)
{
}

ScrollView::ScrollView(View* superview, Window* window, const LG::Rect& frame)
    : View(superview, window, frame)
{
}

void ScrollView::display(const LG::Rect& rect)
{
    LG::Context ctx = graphics_current_context();
    ctx.add_clip(rect);

    display_scroll_indicators(ctx);
}

void ScrollView::do_scroll_animation_step()
{
    int diff = m_animation_target.y() - m_content_offset.y();
    int abs_diff = abs(diff);
    m_scroll_velocity = LFoundation::fast_sqrt(abs_diff);
    m_scroll_velocity *= (((abs_diff / 8)) + 1);
    if (diff < 0) {
        m_scroll_velocity = -m_scroll_velocity;
    }
    do_scroll(0, m_scroll_velocity);
}

void ScrollView::rearm_scroll_animation()
{
    LFoundation::EventLoop::the().add(LFoundation::Timer([this] {
        this->do_scroll_animation_step();
        if (m_content_offset != m_animation_target) {
            this->rearm_scroll_animation();
        } else {
            m_has_timer = false;
        }
    },
        1000 / 60));
}

void ScrollView::do_scroll(int n_x, int n_y)
{
    int x = content_offset().x();
    int y = content_offset().y();
    int max_x = std::max(0, (int)content_size().width() - (int)bounds().width());
    int max_y = std::max(0, (int)content_size().height() - (int)bounds().height());
    content_offset().set_x(std::max(0, std::min(x + n_x, max_x)));
    content_offset().set_y(std::max(0, std::min(y + n_y, max_y)));
    auto me = MouseEvent(m_mouse_location.x(), m_mouse_location.y());
    receive_mouse_move_event(me);
    set_needs_display();
}

void ScrollView::setup_scroll_animation(int wheel_data)
{
    auto get_sign = [](int x) -> int {
        return (x > 0) ? 1 : ((x < 0) ? -1 : 0);
    };

    int dist = abs(m_animation_target.y() - m_content_offset.y());
    if (dist < 10 || get_sign(wheel_data) != get_sign(m_last_scroll_multiplier)) {
        m_last_scroll_multiplier = 10;
    } else if (dist > 30) {
        m_last_scroll_multiplier = 20;
    } else {
        m_last_scroll_multiplier = abs(m_last_scroll_multiplier);
        if (m_last_scroll_multiplier > dist) {
            m_last_scroll_multiplier--;
        } else {
            m_last_scroll_multiplier++;
        }
    }

    if (wheel_data < 0) {
        m_last_scroll_multiplier = -m_last_scroll_multiplier;
    }

    int n_x = 0;
    int n_y = m_last_scroll_multiplier;

    int x = m_animation_target.x();
    int y = m_animation_target.y();
    int max_x = std::max(0, (int)content_size().width() - (int)bounds().width());
    int max_y = std::max(0, (int)content_size().height() - (int)bounds().height());
    m_animation_target.set_x(std::max(0, std::min(x + n_x, max_x)));
    m_animation_target.set_y(std::max(0, std::min(y + n_y, max_y)));

    if (!m_has_timer) {
        m_has_timer = true;
        rearm_scroll_animation();
    }
}

View::WheelEventResponse ScrollView::mouse_wheel_event(int wheel_data)
{
    setup_scroll_animation(wheel_data);
    return View::WheelEventResponse::Handled;
}

LG::Point<int> ScrollView::subview_location(const View& subview) const
{
    auto frame_origin = subview.frame().origin();
    frame_origin.offset_by(-m_content_offset);
    return frame_origin;
}

std::optional<View*> ScrollView::subview_at(const LG::Point<int>& point) const
{
    for (auto it = subviews().rbegin(); it != subviews().rend(); it++) {
        View* view = *it;
        auto frame = view->frame();
        frame.offset_by(-m_content_offset);
        if (frame.contains(point)) {
            return view;
        }
    }
    return {};
}

void ScrollView::receive_mouse_move_event(MouseEvent& event)
{
    m_mouse_location = LG::Point<int>(event.x(), event.y());
    if (!is_hovered()) {
        mouse_entered(m_mouse_location);
    }

    foreach_subview([&](View& subview) -> bool {
        auto frame = subview.frame();
        frame.offset_by(-m_content_offset);
        bool event_hits_subview = frame.contains(event.x(), event.y());
        if (subview.is_hovered() && !event_hits_subview) {
            LG::Point<int> point(event.x(), event.y());
            point.offset_by(-frame.origin());
            MouseLeaveEvent mle(point.x(), point.y());
            subview.receive_mouse_leave_event(mle);
        } else if (event_hits_subview) {
            LG::Point<int> point(event.x(), event.y());
            point.offset_by(-frame.origin());
            MouseEvent me(point.x(), point.y());
            subview.receive_mouse_move_event(me);
        }
        return true;
    });

    mouse_moved(m_mouse_location);
    Responder::receive_mouse_move_event(event);
}

void ScrollView::receive_display_event(DisplayEvent& event)
{
    event.bounds().intersect(bounds());
    display(event.bounds());
    foreach_subview([&](View& subview) -> bool {
        auto bounds = event.bounds();
        auto frame = subview.frame();
        frame.offset_by(-m_content_offset);
        bounds.intersect(frame);
        if (!bounds.empty()) {
            subview.layer().display(bounds, frame);
            graphics_push_context(Context(subview, frame, Context::RelativeToCurrentContext::Yes));
            bounds.origin().offset_by(-frame.origin());
            DisplayEvent own_event(bounds);
            subview.receive_display_event(own_event);
            graphics_pop_context();
        }
        return true;
    });
    did_display(event.bounds());

    if (!has_superview()) {
        // Only superview sends invalidate_message to server.
        bool success = send_invalidate_message_to_server(event.bounds());
    }

    Responder::receive_display_event(event);
}

void ScrollView::recalc_content_props()
{
    int max_width = 0;
    int max_height = 0;

    for (auto* view : subviews()) {
        max_width = std::max(max_width, view->frame().max_x());
        max_height = std::max(max_height, view->frame().max_y());
    }

    m_content_size.set_width(max_width);
    m_content_size.set_height(max_height);
}

void ScrollView::display_scroll_indicators(LG::Context& ctx)
{
    float ratio = (float)bounds().height() / (float)content_size().height();
    int line_height = bounds().height() * ratio;
    int start_y = content_offset().y() * ratio;
    int start_x = bounds().max_x() - 6;
    ctx.set_fill_color(LG::Color(0x6AAAF4));
    ctx.fill_rounded(LG::Rect(start_x, start_y, 4, line_height), LG::CornerMask(2));
}

} // namespace UI