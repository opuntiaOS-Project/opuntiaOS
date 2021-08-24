/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

namespace LFoundation {

class Event {
public:
    enum Type {
        Invalid = 0,
        FdWaiterRead,
        FdWaiterWrite,
        DeferredInvoke,
        Other,
    };

    explicit Event(int type)
        : m_type(type)
    {
    }

    bool operator==(const Event& other)
    {
        return m_type == other.m_type;
    }

    bool operator!=(const Event& other)
    {
        return m_type != other.m_type;
    }

    ~Event() = default;

    int type() const { return m_type; }

private:
    int m_type;
};

} // namespace LFoundation