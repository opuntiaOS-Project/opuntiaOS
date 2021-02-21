/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "Responder.h"
#include <libg/Color.h>
#include <libg/Point.h>
#include <libg/Rect.h>
#include <std/Vector.h>
#include <syscalls.h>

namespace UI {

class Window;

class View : public Responder {
public:
    friend class Window;

    explicit View(const LG::Rect&);
    View(const LG::Rect&, const LG::Color&);

    template <class T, class... Args>
    T& add_subview(Args&&... args)
    {
        T* subview = new T(args...);
        subview->set_window(window());
        subview->set_superview(this);
        m_subviews.push_back(subview);
        return *subview;
    }

    void remove_from_superview();

    template <typename Callback>
    void foreach_subview(Callback callback) const
    {
        for (int i = 0; i < m_subviews.size(); i++) {
            if (!callback(*m_subviews[i])) {
                return;
            }
        }
    }

    inline View* subview_at(const LG::Point<int>& point) const
    {
        for (int i = subviews().size() - 1; i >= 0; --i) {
            if (subviews()[i]->frame().contains(point)) {
                return subviews()[i];
            }
        }
        return nullptr;
    }

    View* hit_test(const LG::Point<int>& point);

    inline const LG::Rect& frame() const { return m_frame; }
    inline const LG::Rect& bounds() const { return m_bounds; }
    inline void set_width(size_t x) { m_frame.set_width(x), m_bounds.set_width(x), set_needs_display(); }
    inline void set_height(size_t x) { m_frame.set_height(x), m_bounds.set_height(x), set_needs_display(); }

    LG::Rect frame_in_window();

    inline Window* window() { return m_window; }
    inline bool has_superview() { return m_superview; }
    inline View* superview() { return m_superview; }
    inline Vector<View*>& subviews() { return m_subviews; }
    inline const Vector<View*>& subviews() const { return m_subviews; }

    void set_needs_display(const LG::Rect&);
    inline void set_needs_display() { set_needs_display(bounds()); }

    inline bool is_hovered() const { return m_hovered; }

    inline void set_focusable(bool val) { m_focusable = val; }
    inline bool is_focusable() const { return m_focusable; }

    virtual void display(const LG::Rect& rect);
    virtual void did_display(const LG::Rect& rect);

    virtual void hover_begin(const LG::Point<int>& location);
    virtual void hover_end();

    virtual void click_began(const LG::Point<int>& location);
    virtual void click_ended();

    virtual void receive_mouse_move_event(MouseEvent&) override;
    virtual void receive_mouse_action_event(MouseActionEvent&) override;
    virtual void receive_mouse_leave_event(MouseLeaveEvent&) override;
    virtual void receive_keyup_event(KeyUpEvent&) override;
    virtual void receive_keydown_event(KeyDownEvent&) override;
    virtual void receive_display_event(DisplayEvent&) override;

    inline LG::Color& background_color() { return m_background_color; }
    inline const LG::Color& background_color() const { return m_background_color; }
    virtual inline void set_background_color(const LG::Color& background_color)
    {
        m_background_color = background_color;
        set_needs_display();
    }

private:
    void set_window(Window* window) { m_window = window; }
    void set_superview(View* superview) { m_superview = superview; }

    View* m_superview { nullptr };
    Window* m_window { nullptr };
    Vector<View*> m_subviews;
    LG::Rect m_frame;
    LG::Rect m_bounds;

    bool m_active { false };
    bool m_hovered { false };
    bool m_focusable { false };

    LG::Color m_background_color { LG::Color::White };
};

}