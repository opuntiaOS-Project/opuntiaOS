/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <libfoundation/Event.h>
#include <libipc/Message.h>
#include <sys/types.h>
#include <syscalls.h>

class WSEvent : public LFoundation::Event {
public:
    enum Type {
        Invalid = 0x1000,
        MouseEvent,
        KeyboardEvent,
        SendEvent,
        Other,
    };

    WSEvent(int type)
        : Event(type)
    {
    }

    ~WSEvent() = default;
};

struct MousePacket {
    int16_t x_offset;
    int16_t y_offset;
    uint32_t button_states;
};

struct KeyboardPacket {
    uint32_t key;
};

class MouseEvent : public WSEvent {
public:
    MouseEvent(const MousePacket& packet)
        : WSEvent(WSEvent::Type::MouseEvent)
        , m_packet(packet)
    {
    }

    ~MouseEvent() = default;

    const MousePacket& packet() const { return m_packet; }
    MousePacket& packet() { return m_packet; }

private:
    MousePacket m_packet;
};

class KeyboardEvent : public WSEvent {
public:
    KeyboardEvent(const KeyboardPacket& packet)
        : WSEvent(WSEvent::Type::KeyboardEvent)
        , m_packet(packet)
    {
    }

    ~KeyboardEvent() = default;

    const KeyboardPacket& packet() const { return m_packet; }
    KeyboardPacket& packet() { return m_packet; }

private:
    KeyboardPacket m_packet;
};

class SendEvent : public WSEvent {
public:
    SendEvent(Message* msg)
        : WSEvent(WSEvent::Type::SendEvent)
        , m_message(msg)
    {
    }

    SendEvent(SendEvent&& ev)
        : WSEvent(WSEvent::Type::SendEvent)
        , m_message(move(ev.m_message))
    {
    }

    SendEvent& operator=(SendEvent&& ev)
    {
        m_message = move(ev.m_message);
        return *this;
    }

    ~SendEvent()
    {
    }

    const UniquePtr<Message>& message() const { return m_message; }
    UniquePtr<Message>& message() { return m_message; }

private:
    UniquePtr<Message> m_message;
};