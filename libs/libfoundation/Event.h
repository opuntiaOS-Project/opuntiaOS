/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
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

    Event(int type)
        : m_type(type)
    {
    }

    ~Event() = default;

    int type() { return m_type; }

private:
    int m_type;
};

}