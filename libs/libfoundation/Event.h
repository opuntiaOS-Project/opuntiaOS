/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
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

    ~Event() { }

    int type() { return m_type; }

private:
    int m_type;
};

}