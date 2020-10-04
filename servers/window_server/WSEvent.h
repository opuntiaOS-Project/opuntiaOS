/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once

#include <libfoundation/Event.h>
#include <sys/types.h>

class WSEvent : public LFoundation::Event {
public:
    enum Type {
        Invalid = 0x1000,
        MouseEvent,
        Other,
    };

    WSEvent(int type)
        : Event(type)
    {
    }

    ~WSEvent() { }
};

struct MousePacket {
    int16_t x_offset;
    int16_t y_offset;
    uint32_t button_states;
};

class MouseEvent : public WSEvent {
public:
    MouseEvent(const MousePacket& packet)
        : WSEvent(WSEvent::Type::MouseEvent)
        , m_packet(packet)
    {
    }

    ~MouseEvent() { }

    const MousePacket& packet() const { return m_packet; }
    MousePacket& packet() { return m_packet; }

private:
    MousePacket m_packet;
};