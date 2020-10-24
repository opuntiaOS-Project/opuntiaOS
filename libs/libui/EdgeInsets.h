/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once

namespace UI {

class EdgeInsect {
public:
    EdgeInsect() = default;
    EdgeInsect(int top, int left, int bottom, int right)
        : m_top(top)
        , m_left(left)
        , m_bottom(bottom)
        , m_right(right)
    {
    }

    inline int top() const { return m_top; }
    inline int left() const { return m_left; }
    inline int bottom() const { return m_bottom; }
    inline int right() const { return m_right; }

private:
    int m_top { 0 };
    int m_bottom { 0 };
    int m_left { 0 };
    int m_right { 0 };
};

}