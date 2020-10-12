/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once
#include "Responder.h"
#include <libg/Point.h>
#include <libg/Rect.h>
#include <std/Vector.h>
#include <syscalls.h>

namespace UI {

class View : public Responder {
public:
    View(const LG::Rect&);

    void add_subview(View* subview);
    void remove_from_superview();

    template <typename Callback>
    void foreach_subview(Callback callback) const
    {
        for (int i = 0; i < m_subviews.size(); i++) {
            if (callback(*m_subviews[i])) {
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

    View* superview() { return m_superview; }
    inline Vector<View*>& subviews() { return m_subviews; }
    inline const Vector<View*>& subviews() const { return m_subviews; }

    virtual void receive_mouse_event(UniquePtr<MouseEvent>) override;

private:
    void set_superview(View* superview) { m_superview = superview; }

    View* m_superview { nullptr };
    Vector<View*> m_subviews;
    LG::Rect m_frame;
    LG::Rect m_bounds;
};

}