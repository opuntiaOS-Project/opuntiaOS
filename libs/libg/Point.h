/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once

#include <libcxx/sys/types.h>

namespace LG {

template <typename T>
class Point {
public:
    Point(T x, T y)
        : m_x(x)
        , m_y(y)
    {
    }
    ~Point() { }

    inline void set_x(int x) { m_x = x; }
    inline void set_y(int y) { m_y = y; }

    inline T x() const { return m_x; }
    inline T y() const { return m_y; }

    inline void offset_by(int x, int y) { m_x += x, m_y += y; }
    inline void offset_by(const Point& p) { offset_by(p.x(), p.y()); }

    Point operator-() const { return { -m_x, -m_y }; }
    Point operator-(const Point& p) const { return { m_x - p.m_x, m_y - p.m_y }; }
    Point& operator-=(const Point& p)
    {
        m_x -= p.m_x;
        m_y -= p.m_y;
        return *this;
    }

    Point& operator+=(const Point& p)
    {
        m_x += p.m_x;
        m_y += p.m_y;
        return *this;
    }
    Point operator+(const Point& p) const { return { m_x + p.m_x, m_y + p.m_y }; }

private:
    T m_x;
    T m_y;
};

}