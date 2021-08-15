/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

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

// void ScrollView::display(const LG::Rect& rect)
// {
//     Context ctx(*this);
//     ctx.add_clip(rect);

// }

void ScrollView::mouse_wheel_event(int wheel_data)
{
    m_content_offset.offset_by(0, wheel_data * 10);
    set_needs_display();
}

void ScrollView::receive_display_event(DisplayEvent& event)
{
    event.bounds().intersect(bounds());
    display(event.bounds());
    foreach_subview([&](View& subview) -> bool {
        auto bounds = event.bounds();
        auto frame = subview.frame();
        frame.offset_by(m_content_offset);
        bounds.intersect(frame);
        if (!bounds.empty()) {
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

} // namespace UI