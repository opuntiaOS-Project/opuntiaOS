/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libg/Color.h>
#include <libg/Point.h>
#include <libg/Rect.h>
#include <libui/Constraint.h>
#include <libui/EdgeInsets.h>
#include <libui/Responder.h>
#include <vector>

#define UI_OBJECT() friend class View

namespace UI {

struct SafeArea {
    static const int Top = 8;
    static const int Bottom = 8;
    static const int Left = 8;
    static const int Right = 8;
};

class Window;

class View : public Responder {
public:
    friend class Window;

    ~View() = default;

    template <class T, class... Args>
    T& add_subview(Args&&... args)
    {
        T* subview = new T(this, args...);
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

    inline void turn_on_constraint_based_layout(bool b) { m_constraint_based_layout = b; }
    void add_constraint(const Constraint& constraint) { m_constrints.push_back(constraint); }

    virtual void layout_subviews();
    inline void set_needs_layout()
    {
        send_layout_message(*window(), this);
        set_needs_display();
    }

    LG::Rect frame_in_window();

    inline Window* window() { return m_window; }
    inline bool has_superview() { return m_superview; }
    inline View* superview() { return m_superview; }
    inline std::vector<View*>& subviews() { return m_subviews; }
    inline const std::vector<View*>& subviews() const { return m_subviews; }

    void set_needs_display(const LG::Rect&);
    inline void set_needs_display() { set_needs_display(bounds()); }

    inline bool is_hovered() const { return m_hovered; }

    inline void set_focusable(bool val) { m_focusable = val; }
    inline bool is_focusable() const { return m_focusable; }

    virtual void display(const LG::Rect& rect);
    virtual void did_display(const LG::Rect& rect);

    virtual void mouse_moved(const LG::Point<int>& new_location);
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
    virtual bool receive_layout_event(const LayoutEvent&) override;

    inline LG::Color& background_color() { return m_background_color; }
    inline const LG::Color& background_color() const { return m_background_color; }
    virtual inline void set_background_color(const LG::Color& background_color)
    {
        m_background_color = background_color;
        set_needs_display();
    }

protected:
    View(View* superview, const LG::Rect&);

private:
    inline LG::Rect& frame() { return m_frame; }

    void set_window(Window* window) { m_window = window; }
    void set_superview(View* superview) { m_superview = superview; }
    inline void constraint_interpreter(const Constraint& constraint);

    View* m_superview { nullptr };
    Window* m_window { nullptr };
    std::vector<View*> m_subviews;
    LG::Rect m_frame;
    LG::Rect m_bounds;

    bool m_constraint_based_layout { false };
    std::vector<Constraint> m_constrints {};

    bool m_active { false };
    bool m_hovered { false };
    bool m_focusable { false };

    LG::Color m_background_color { LG::Color::White };
};

inline void View::constraint_interpreter(const Constraint& constraint)
{
    auto get_rel_item_attribute = [&]() {
        // Constraints could be added between a view and the view's superview or another view with the same superview.
        //
        // If it's the case view and the view's superview, it has to take attributes from bound to calculate the right
        // view's posistion within this superview.
        if (constraint.rel_item() == constraint.item()->superview()) {
            return Constraint::get_attribute<int>(constraint.rel_item()->bounds(), constraint.rel_attribute());
        } else {
            return Constraint::get_attribute<int>(constraint.rel_item()->frame(), constraint.rel_attribute());
        }
    };

    auto calc_new_value = [&]() {
        return constraint.multiplier() * get_rel_item_attribute() + constraint.constant();
    };

    switch (constraint.attribute()) {
    case Constraint::Attribute::Top:
        Constraint::set_attribute<Constraint::Attribute::Top>(constraint.item()->frame(), calc_new_value());
        return;

    case Constraint::Attribute::Bottom:
        Constraint::set_attribute<Constraint::Attribute::Bottom>(constraint.item()->frame(), calc_new_value());
        return;

    case Constraint::Attribute::Left:
        Constraint::set_attribute<Constraint::Attribute::Left>(constraint.item()->frame(), calc_new_value());
        return;

    case Constraint::Attribute::Right:
        Constraint::set_attribute<Constraint::Attribute::Right>(constraint.item()->frame(), calc_new_value());
        return;

    case Constraint::Attribute::CenterX:
        Constraint::set_attribute<Constraint::Attribute::CenterX>(constraint.item()->frame(), calc_new_value());
        return;

    case Constraint::Attribute::CenterY:
        Constraint::set_attribute<Constraint::Attribute::CenterY>(constraint.item()->frame(), calc_new_value());
        return;

    case Constraint::Attribute::Width:
        Constraint::set_attribute<Constraint::Attribute::Width>(constraint.item()->frame(), calc_new_value());
        return;

    case Constraint::Attribute::Height:
        Constraint::set_attribute<Constraint::Attribute::Height>(constraint.item()->frame(), calc_new_value());
        return;

    default:
        break;
    }
}

} // namespace UI