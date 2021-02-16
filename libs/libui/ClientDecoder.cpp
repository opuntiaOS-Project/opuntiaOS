/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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
            uint32_t key = msg.kbd_key();
            key &= 0xEFFFFFFF;
            m_event_loop.add(App::the().window(), new KeyUpEvent(key));
        } else {
            m_event_loop.add(App::the().window(), new KeyDownEvent(msg.kbd_key()));
        }
    }
    return nullptr;
}

UniquePtr<Message> ClientDecoder::handle(const DisplayMessage& msg)
{
    m_event_loop.add(App::the().window(), new DisplayEvent(msg.rect()));
    return nullptr;
}

UniquePtr<Message> ClientDecoder::handle(const WindowCloseRequestMessage& msg)
{
    // TODO: Currently we support only 1 window per app.
    if (App::the().window().id() == msg.win_id()) {
        m_event_loop.add(App::the(), new WindowCloseRequestEvent(msg.win_id()));
    }
    return nullptr;
}

// Notifiers
UniquePtr<Message> ClientDecoder::handle(const NotifyWindowStatusChangedMessage& msg)
{
    if (App::the().window().id() == msg.win_id()) {
        m_event_loop.add(App::the().window(), new NotifyWindowStatusChangedEvent(msg.changed_window_id(), msg.type()));
    }
    return nullptr;
}

UniquePtr<Message> ClientDecoder::handle(const NotifyWindowIconChangedMessage& msg)
{
    if (App::the().window().id() == msg.win_id()) {
        m_event_loop.add(App::the().window(), new NotifyWindowIconChangedEvent(msg.changed_window_id(), msg.icon_path()));
    }
    return nullptr;
}

}