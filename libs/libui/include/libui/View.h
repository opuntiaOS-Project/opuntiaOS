/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libfoundation/Logger.h>
#include <libg/Color.h>
#include <libg/Point.h>
#include <libg/Rect.h>
#include <libui/Constraint.h>
#include <libui/ContextManager.h>
#include <libui/EdgeInsets.h>
#include <libui/Layer.h>
#include <libui/Responder.h>
#include <optional>
#include <utility>
#include <vector>

#define UI_OBJECT() friend class View

namespace UI {

struct SafeArea {
    static const int Top = 8;
#ifdef TARGET_DESKTOP
    static const int Bottom = 8;
#elif TARGET_MOBILE
    static const int Bottom = 20;
#endif
    static const int Left = 8;
    static const int Right = 8;
};

struct Padding {
    static const int System = 8;
    static const int AfterTitle = 12;
};

class Window;

class View : public Responder {
public:
    friend class Window;

    ~View() = default;

    template <class T, class... Args>
    T& add_subview(Args&&... args)
    {
        T* subview = new T(this, std::forward<Args>(args)...);
        m_subviews.push_back(subview);
        did_add_subview(*subview);
        return *subview;
    }
    virtual void did_add_subview(View& view) { }

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

    virtual std::optional<View*> subview_at(const LG::Point<int>& point) const;
    View& hit_test(const LG::Point<int>& point);

    inline const LG::Rect& frame() const { return m_frame; }
    inline const LG::Rect& bounds() const { return m_bounds; }
    inline LG::Rect& frame() { return m_frame; }
    inline LG::Rect& bounds() { return m_bounds; }
    inline LG::Point<int> center() { return LG::Point<int>(frame().mid_x(), frame().mid_y()); }
    inline void set_width(size_t x) { m_frame.set_width(x), m_bounds.set_width(x), set_needs_display(); }
    inline void set_height(size_t x) { m_frame.set_height(x), m_bounds.set_height(x), set_needs_display(); }

    inline void turn_on_constraint_based_layout(bool b) { m_constraint_based_layout = b; }
    void add_constraint(const Constraint& constraint) { m_constrints.push_back(constraint); }
    const std::vector<UI::Constraint>& constraints() const { return m_constrints; }

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
    inline bool is_active() const { return m_active; }

    inline void set_focusable(bool val) { m_focusable = val; }
    inline bool is_focusable() const { return m_focusable; }

    inline Layer& layer() { return m_layer; }
    inline const Layer& layer() const { return m_layer; }

    virtual void display(const LG::Rect& rect);
    virtual void did_display(const LG::Rect& rect);

    virtual void mouse_moved(const LG::Point<int>& new_location);
    virtual void mouse_wheel_event(int wheel_data);
    virtual void mouse_entered(const LG::Point<int>& location);
    virtual void mouse_exited();

    virtual void mouse_down(const LG::Point<int>& location);
    virtual void mouse_up();

    virtual void receive_mouse_move_event(MouseEvent&) override;
    virtual void receive_mouse_action_event(MouseActionEvent&) override;
    virtual void receive_mouse_leave_event(MouseLeaveEvent&) override;
    virtual void receive_mouse_wheel_event(MouseWheelEvent&) override;
    virtual void receive_keyup_event(KeyUpEvent&) override;
    virtual void receive_keydown_event(KeyDownEvent&) override;
    virtual void receive_display_event(DisplayEvent&) override;
    virtual bool receive_layout_event(const LayoutEvent&, bool force_layout_if_not_target = false) override;

    inline LG::Color& background_color() { return m_background_color; }
    inline const LG::Color& background_color() const { return m_background_color; }
    inline void set_background_color(const LG::Color& background_color) { m_background_color = background_color, set_needs_display(); }

protected:
    View(View* superview, const LG::Rect&);
    View(View* superview, Window* window, const LG::Rect&);

    inline void set_hovered(bool value) { m_hovered = value; }
    inline void set_active(bool value) { m_active = value; }

    virtual std::optional<LG::Point<int>> subview_location(const View& subview) const;

    template <Constraint::Attribute attr>
    inline void add_interpreted_constraint_to_mask() { m_applied_constraints_mask |= (1 << (int)attr); }

    template <Constraint::Attribute attr>
    inline bool has_interpreted_constraint_in_mask() { return (m_applied_constraints_mask & (1 << (int)attr)); }

    inline void constraint_interpreter(const Constraint& constraint);

private:
    void set_window(Window* window) { m_window = window; }
    void set_superview(View* superview) { m_superview = superview; }

    View* m_superview { nullptr };
    Window* m_window { nullptr };
    std::vector<View*> m_subviews;
    LG::Rect m_frame;
    LG::Rect m_bounds;

    bool m_constraint_based_layout { false };
    std::vector<Constraint> m_constrints {};
    uint32_t m_applied_constraints_mask { 0 }; // Constraints applied to this view;

    bool m_active { false };
    bool m_hovered { false };
    bool m_focusable { false };

    LG::Color m_background_color { LG::Color::White };
    Layer m_layer;
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
        constraint.item()->add_interpreted_constraint_to_mask<Constraint::Attribute::Top>();
        return;

    case Constraint::Attribute::Bottom: {
        uint32_t value = calc_new_value();
        if (!constraint.rel_item()) {
            value = constraint.item()->superview()->bounds().max_y() - constraint.constant();
        }

        if (constraint.item()->has_interpreted_constraint_in_mask<Constraint::Attribute::Top>()) {
            uint32_t res = value - constraint.item()->frame().min_y();
            Constraint::set_attribute<Constraint::Attribute::Height>(constraint.item()->frame(), res);
            Constraint::set_attribute<Constraint::Attribute::Height>(constraint.item()->bounds(), res);
        } else {
            Constraint::set_attribute<Constraint::Attribute::Bottom>(constraint.item()->frame(), value);
        }
        constraint.item()->add_interpreted_constraint_to_mask<Constraint::Attribute::Bottom>();
        return;
    }
    case Constraint::Attribute::Left:
        Constraint::set_attribute<Constraint::Attribute::Left>(constraint.item()->frame(), calc_new_value());
        constraint.item()->add_interpreted_constraint_to_mask<Constraint::Attribute::Left>();
        return;

    case Constraint::Attribute::Right: {
        uint32_t value = calc_new_value();
        if (!constraint.rel_item()) {
            value = constraint.item()->superview()->bounds().max_x() - constraint.constant();
        }

        if (constraint.item()->has_interpreted_constraint_in_mask<Constraint::Attribute::Left>()) {
            uint32_t res = value - constraint.item()->frame().min_x();
            Constraint::set_attribute<Constraint::Attribute::Width>(constraint.item()->frame(), res);
            Constraint::set_attribute<Constraint::Attribute::Width>(constraint.item()->bounds(), res);
        } else {
            Constraint::set_attribute<Constraint::Attribute::Right>(constraint.item()->frame(), value);
        }
        constraint.item()->add_interpreted_constraint_to_mask<Constraint::Attribute::Right>();
        return;
    }

    case Constraint::Attribute::CenterX:
        Constraint::set_attribute<Constraint::Attribute::CenterX>(constraint.item()->frame(), calc_new_value());
        constraint.item()->add_interpreted_constraint_to_mask<Constraint::Attribute::CenterX>();
        return;

    case Constraint::Attribute::CenterY:
        Constraint::set_attribute<Constraint::Attribute::CenterY>(constraint.item()->frame(), calc_new_value());
        constraint.item()->add_interpreted_constraint_to_mask<Constraint::Attribute::CenterY>();
        return;

    case Constraint::Attribute::Width:
        Constraint::set_attribute<Constraint::Attribute::Width>(constraint.item()->frame(), calc_new_value());
        Constraint::set_attribute<Constraint::Attribute::Width>(constraint.item()->bounds(), calc_new_value());
        constraint.item()->add_interpreted_constraint_to_mask<Constraint::Attribute::Width>();
        return;

    case Constraint::Attribute::Height:
        Constraint::set_attribute<Constraint::Attribute::Height>(constraint.item()->frame(), calc_new_value());
        Constraint::set_attribute<Constraint::Attribute::Height>(constraint.item()->bounds(), calc_new_value());
        constraint.item()->add_interpreted_constraint_to_mask<Constraint::Attribute::Height>();
        return;

    default:
        break;
    }
}

} // namespace UI