/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libg/Color.h>
#include <libui/CollectionView.h>
#include <libui/Context.h>
#include <utility>

namespace UI {

CollectionView::CollectionView(View* superview, const LG::Rect& frame)
    : ScrollView(superview, frame)
{
}

CollectionView::CollectionView(View* superview, Window* window, const LG::Rect& frame)
    : ScrollView(superview, window, frame)
{
}

void CollectionView::display(const LG::Rect& rect)
{
    LG::Context ctx = graphics_current_context();
    ctx.add_clip(rect);

    display_scroll_indicators(ctx);
}

void CollectionView::mouse_entered(const LG::Point<int>& location)
{
    set_hovered(true);
}

void CollectionView::mouse_exited()
{
    set_hovered(false);
}

CollectionView::PrefetchStatus CollectionView::prefetch_row_forward(int id)
{
    size_t cached_lines = m_first_offscreen_line + m_next_cached_views.size();
    if (id < cached_lines) {
        return PrefetchStatus::Success;
    }

    // See comment on CollectionViewRowStreamer
    View* view = m_data_source((int)id);
    if (!view) {
        return PrefetchStatus::EndOfStream;
    }

    m_next_cached_views.push_back(view);
    view->frame().set_origin(m_next_frame_origin);
    view->set_needs_layout();

    m_next_frame_origin.set_y(m_next_frame_origin.y() + view->bounds().height());
    content_size().set_height(content_size().height() + view->bounds().height());

    return PrefetchStatus::Success;
}

void CollectionView::prefetch_data_forward()
{
    if (!m_data_source) {
        return;
    }

    // Not initialized, so prefetching while data is on screen.
    if (content_size().height() < bounds().height()) {
        for (;; m_first_offscreen_line++) {
            PrefetchStatus full_line_prefetched = prefetch_row_forward(m_first_offscreen_line);
            if (full_line_prefetched == PrefetchStatus::EndOfStream) {
                break;
            }

            m_views_on_screen.push_back(m_next_cached_views.front());
            m_next_cached_views.pop_front();

            if (!bounds().contains(m_next_frame_origin)) {
                m_first_offscreen_line++;
                // Exiting here since the next frame is out of screen.
                return;
            }
        }
    }

    const int cache_depth = 4;
    for (int i = 0; i < cache_depth; i++) {
        PrefetchStatus full_line_prefetched = prefetch_row_forward(m_first_offscreen_line + i);
        if (full_line_prefetched == PrefetchStatus::EndOfStream) {
            break;
        }
    }
}

void CollectionView::after_scroll_backward()
{

    if (!m_prev_cached_views.empty()) {
        View* back_view = m_prev_cached_views.back();
        auto back_view_frame = back_view->frame();
        back_view_frame.offset_by(-content_offset());
        if (bounds().intersects(back_view_frame)) {
            m_views_on_screen.push_front(m_prev_cached_views.back());
            m_prev_cached_views.pop_back();
            m_first_onscreen_line--;
        }
    }

    if (!m_views_on_screen.empty()) {
        View* back_view = m_views_on_screen.back();
        auto back_view_frame = back_view->frame();
        back_view_frame.offset_by(-content_offset());
        if (!bounds().intersects(back_view_frame)) {
            m_next_cached_views.push_front(m_views_on_screen.back());
            m_views_on_screen.pop_back();
            m_first_offscreen_line--;
        }
    }
}

void CollectionView::after_scroll_forward()
{
    if (!m_views_on_screen.empty()) {
        View* front_view = m_views_on_screen.front();
        auto front_view_frame = front_view->frame();
        front_view_frame.offset_by(-content_offset());
        if (!bounds().intersects(front_view_frame)) {
            m_prev_cached_views.push_back(m_views_on_screen.front());
            m_views_on_screen.pop_front();
            m_first_onscreen_line++;
        }
    }

    if (!m_next_cached_views.empty()) {
        View* front_view = m_next_cached_views.front();
        auto front_view_frame = front_view->frame();
        front_view_frame.offset_by(-content_offset());
        if (bounds().intersects(front_view_frame)) {
            m_views_on_screen.push_back(m_next_cached_views.front());
            m_next_cached_views.pop_front();
            m_first_offscreen_line++;
        }
    }
}

void CollectionView::add_to_bounds_size()
{
    // content_size() +=
}

} // namespace UI