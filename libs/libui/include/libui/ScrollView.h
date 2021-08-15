/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
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
    inline const LG::Point<int>& content_offset() const { return m_content_offset; }
    inline void set_content_offset(const LG::Point<int>& offset) { m_content_offset = offset; }

    virtual void mouse_wheel_event(int wheel_data) override;
    virtual void receive_display_event(DisplayEvent&) override;

protected:
    ScrollView(View* superview, const LG::Rect&);
    ScrollView(View* superview, Window* window, const LG::Rect& frame);

    void display_scroll_indicators();

private:
    void recalc_content_props();

    LG::Size m_content_size {};
    LG::Point<int> m_content_offset {};
};

} // namespace UI