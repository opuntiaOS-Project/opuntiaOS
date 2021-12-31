/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <libfoundation/Event.h>
#include <libipc/Message.h>
#include <memory>
#include <sys/types.h>

namespace WinServer {

class Event : public LFoundation::Event {
public:
    enum Type {
        Invalid = 0x1000,
        MouseEvent,
        KeyboardEvent,
        SendEvent,
        Other,
    };

    explicit Event(int type)
        : LFoundation::Event(type)
    {
    }

    ~Event() = default;
};

struct MousePacket {
    int16_t x_offset;
    int16_t y_offset;
    uint16_t button_states;
    int16_t wheel_data;
};

struct KeyboardPacket {
    uint32_t key;
};

class MouseEvent : public WinServer::Event {
public:
    explicit MouseEvent(const MousePacket& packet)
        : WinServer::Event(WinServer::Event::Type::MouseEvent)
        , m_packet(packet)
    {
    }

    ~MouseEvent() = default;

    const MousePacket& packet() const { return m_packet; }
    MousePacket& packet() { return m_packet; }

private:
    MousePacket m_packet;
};

class KeyboardEvent : public WinServer::Event {
public:
    explicit KeyboardEvent(const KeyboardPacket& packet)
        : WinServer::Event(WinServer::Event::Type::KeyboardEvent)
        , m_packet(packet)
    {
    }

    ~KeyboardEvent() = default;

    const KeyboardPacket& packet() const { return m_packet; }
    KeyboardPacket& packet() { return m_packet; }

private:
    KeyboardPacket m_packet;
};

class SendEvent : public WinServer::Event {
public:
    explicit SendEvent(Message* msg)
        : WinServer::Event(WinServer::Event::Type::SendEvent)
        , m_message(msg)
    {
    }

    SendEvent(SendEvent&& ev)
        : WinServer::Event(WinServer::Event::Type::SendEvent)
        , m_message(std::move(ev.m_message))
    {
    }

    SendEvent& operator=(SendEvent&& ev)
    {
        m_message = std::move(ev.m_message);
        return *this;
    }

    ~SendEvent() = default;

    const std::unique_ptr<Message>& message() const { return m_message; }
    std::unique_ptr<Message>& message() { return m_message; }

private:
    std::unique_ptr<Message> m_message;
};

} // namespace WinServer