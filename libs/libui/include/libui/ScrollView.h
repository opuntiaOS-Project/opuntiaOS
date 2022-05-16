/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libg/Size.h>
#include <libui/Constants/Layout.h>
#include <libui/EdgeInsets.h>
#include <libui/View.h>
#include <string>
#include <utility>

namespace UI {

class ScrollView : public View {
    UI_OBJECT();

public:
    ~ScrollView() = default;

    inline const LG::Size& content_size() const { return m_content_size; }
    inline LG::Size& content_size() { return m_content_size; }
    inline const LG::Point<int>& content_offset() const { return m_content_offset; }
    inline LG::Point<int>& content_offset() { return m_content_offset; }

    virtual std::optional<View*> subview_at(const LG::Point<int>& point) const override;

    virtual void display(const LG::Rect& rect) override;
    virtual WheelEventResponse mouse_wheel_event(int wheel_data) override;
    virtual void receive_mouse_move_event(MouseEvent&) override;
    virtual void receive_display_event(DisplayEvent&) override;

protected:
    ScrollView(View* superview, const LG::Rect&);
    ScrollView(View* superview, Window* window, const LG::Rect& frame);

    void display_scroll_indicators(LG::Context&);

    // The location of a subview relativly to its superview could
    // differ from it's frame() (e.g when scrolling), to determine
    // the right location we ask the superview to return it.
    virtual LG::Point<int> subview_location(const View& subview) const override;

private:
    void setup_scroll_animation(int wheel_data);
    void do_scroll_animation_step();
    void rearm_scroll_animation();
    void do_scroll(int n_x, int n_y);
    void recalc_content_props();

    LG::Size m_content_size {};
    LG::Point<int> m_content_offset {};
    LG::Point<int> m_mouse_location {};

    bool m_has_timer { false };
    size_t m_scroll_velocity { 0 };
    size_t m_last_scroll_multiplier { 0 };
    LG::Point<int> m_animation_target { 0, 0 };
};

} // namespace UI