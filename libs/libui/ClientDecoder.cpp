/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#include "ClientDecoder.h"
#include "App.h"
#include "Event.h"
#include <syscalls.h>

namespace UI {

ClientDecoder::ClientDecoder()
    : m_event_loop(LFoundation::EventLoop::the())
{
}

UniquePtr<Message> ClientDecoder::handle(const MouseMoveMessage& msg)
{
    if (App::the().window().id() == msg.win_id()) {
        m_event_loop.add(App::the().window(), new MouseEvent(msg.x(), msg.y()));
    }
    return nullptr;
}

UniquePtr<Message> ClientDecoder::handle(const MouseActionMessage& msg)
{
    if (App::the().window().id() == msg.win_id()) {
        m_event_loop.add(App::the().window(), new MouseActionEvent((MouseActionType)msg.type(), msg.x(), msg.y()));
    }
    return nullptr;
}

UniquePtr<Message> ClientDecoder::handle(const MouseLeaveMessage& msg)
{
    if (App::the().window().id() == msg.win_id()) {
        m_event_loop.add(App::the().window(), new MouseLeaveEvent(msg.x(), msg.y()));
    }
    return nullptr;
}

UniquePtr<Message> ClientDecoder::handle(const KeyboardMessage& msg)
{
    if (App::the().window().id() == msg.win_id()) {
        // Checking if the key is down or up
        if (msg.kbd_key() >> 31) {
            m_event_loop.add(App::the().window(), new KeyUpEvent(msg.key()));
        } else {
            m_event_loop.add(App::the().window(), new KeyDownEvent(msg.key()));
        }
    }
    return nullptr;
}

UniquePtr<Message> ClientDecoder::handle(const DisplayMessage& msg)
{
    m_event_loop.add(App::the().window(), new DisplayEvent(msg.rect()));
    return nullptr;
}

}