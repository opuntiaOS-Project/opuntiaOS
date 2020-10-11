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

    inline T x() const { return m_x; }
    inline T y() const { return m_y; }

private:
    T m_x;
    T m_y;
};

}