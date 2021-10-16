/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <libui/App.h>
#include <libui/ClientDecoder.h>
#include <libui/Event.h>

namespace UI {

ClientDecoder::ClientDecoder()
    : m_event_loop(LFoundation::EventLoop::the())
{
}

std::unique_ptr<Message> ClientDecoder::handle(MouseMoveMessage& msg)
{
    if (App::the().window().id() == msg.win_id()) {
        m_event_loop.add(App::the().window(), new MouseEvent(msg.x(), msg.y()));
    }
    return nullptr;
}

std::unique_ptr<Message> ClientDecoder::handle(MouseActionMessage& msg)
{
    if (App::the().window().id() == msg.win_id()) {
        m_event_loop.add(App::the().window(), new MouseActionEvent((MouseActionType)msg.type(), msg.x(), msg.y()));
    }
    return nullptr;
}

std::unique_ptr<Message> ClientDecoder::handle(MouseLeaveMessage& msg)
{
    if (App::the().window().id() == msg.win_id()) {
        m_event_loop.add(App::the().window(), new MouseLeaveEvent(msg.x(), msg.y()));
    }
    return nullptr;
}

std::unique_ptr<Message> ClientDecoder::handle(MouseWheelMessage& msg)
{
    if (App::the().window().id() == msg.win_id()) {
        m_event_loop.add(App::the().window(), new MouseWheelEvent(msg.x(), msg.y(), msg.wheel_data()));
    }
    return nullptr;
}

std::unique_ptr<Message> ClientDecoder::handle(KeyboardMessage& msg)
{
    if (App::the().window().id() == msg.win_id()) {
        // Checking if the key is down or up
        if (msg.kbd_key() >> 31) {
            uint32_t key = msg.kbd_key();
            key &= 0x7FFFFFFF;
            m_event_loop.add(App::the().window(), new KeyUpEvent(key));
        } else {
            m_event_loop.add(App::the().window(), new KeyDownEvent(msg.kbd_key()));
        }
    }
    return nullptr;
}

std::unique_ptr<Message> ClientDecoder::handle(DisplayMessage& msg)
{
    m_event_loop.add(App::the().window(), new DisplayEvent(msg.rect()));
    return nullptr;
}

std::unique_ptr<Message> ClientDecoder::handle(WindowCloseRequestMessage& msg)
{
    // TODO: Currently we support only 1 window per app.
    if (App::the().window().id() == msg.win_id()) {
        m_event_loop.add(App::the(), new WindowCloseRequestEvent(msg.win_id()));
    }
    return nullptr;
}

std::unique_ptr<Message> ClientDecoder::handle(ResizeMessage& msg)
{
    if (App::the().window().id() == msg.win_id()) {
        m_event_loop.add(App::the().window(), new ResizeEvent(msg.win_id(), msg.rect()));
    }
    return nullptr;
}

std::unique_ptr<Message> ClientDecoder::handle(MenuBarActionMessage& msg)
{
    if (App::the().window().id() == msg.win_id()) {
        m_event_loop.add(App::the().window(), new MenuBarActionEvent(msg.win_id(), msg.menu_id(), msg.item_id()));
    }
    return nullptr;
}

std::unique_ptr<Message> ClientDecoder::handle(PopupActionMessage& msg)
{
    if (App::the().window().id() == msg.win_id()) {
        m_event_loop.add(App::the().window(), new PopupActionEvent(msg.win_id(), msg.menu_id(), msg.item_id()));
    }
    return nullptr;
}

// Notifiers
std::unique_ptr<Message> ClientDecoder::handle(NotifyWindowCreateMessage& msg)
{
    if (App::the().window().id() == msg.win_id()) {
        m_event_loop.add(App::the().window(), new NotifyWindowCreateEvent(msg.bundle_id().move_string(), msg.icon_path().move_string(), msg.changed_window_id()));
    }
    return nullptr;
}

std::unique_ptr<Message> ClientDecoder::handle(NotifyWindowStatusChangedMessage& msg)
{
    if (App::the().window().id() == msg.win_id()) {
        m_event_loop.add(App::the().window(), new NotifyWindowStatusChangedEvent(msg.changed_window_id(), msg.type()));
    }
    return nullptr;
}

std::unique_ptr<Message> ClientDecoder::handle(NotifyWindowIconChangedMessage& msg)
{
    if (App::the().window().id() == msg.win_id()) {
        m_event_loop.add(App::the().window(), new NotifyWindowIconChangedEvent(msg.changed_window_id(), msg.icon_path().move_string()));
    }
    return nullptr;
}

std::unique_ptr<Message> ClientDecoder::handle(NotifyWindowTitleChangedMessage& msg)
{
    if (App::the().window().id() == msg.win_id()) {
        m_event_loop.add(App::the().window(), new NotifyWindowTitleChangedEvent(msg.changed_window_id(), msg.title().move_string()));
    }
    return nullptr;
}

} // namespace UI