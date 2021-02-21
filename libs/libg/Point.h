/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <libcxx/sys/types.h>
#include <libipc/Decodable.h>
#include <libipc/Encodable.h>
#include <libipc/Encoder.h>

namespace LG {

template <typename T>
class Point : public Encodable<Point<T>>, public Decodable<Point<T>> {
public:
    Point() = default;

    Point(T x, T y)
        : m_x(x)
        , m_y(y)
    {
    }
    ~Point() = default;

    inline void set(const Point& p) { m_x = p.x(), m_y = p.y(); }
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

    bool operator==(const Point& p) const
    {
        return m_x == p.m_x && m_y == p.m_y;
    }

    bool operator!=(const Point& p) const
    {
        return !(*this == p);
    }

    void encode(EncodedMessage& buf) const override
    {
        Encoder::append(buf, m_x);
        Encoder::append(buf, m_y);
    }

    void decode(const char* buf, size_t& offset) override
    {
        Encoder::decode(buf, offset, m_x);
        Encoder::decode(buf, offset, m_y);
    }

private:
    T m_x {};
    T m_y {};
};

}