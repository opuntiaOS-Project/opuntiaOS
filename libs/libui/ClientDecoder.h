/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once
#include "../servers/window_server/WSConnection.h"
#include <libfoundation/EventLoop.h>

namespace UI {

class App;

class ClientDecoder : public WindowClientDecoder {
public:
    ClientDecoder();
    ~ClientDecoder() { }

    using WindowClientDecoder::handle;
    virtual UniquePtr<Message> handle(const MouseMoveMessage& msg) override;
    virtual UniquePtr<Message> handle(const MouseActionMessage& msg) override;
    virtual UniquePtr<Message> handle(const MouseLeaveMessage& msg) override;
    virtual UniquePtr<Message> handle(const KeyboardMessage& msg) override;
    virtual UniquePtr<Message> handle(const DisplayMessage& msg) override;
    virtual UniquePtr<Message> handle(const WindowCloseRequestMessage& msg) override;

    // Notifiers
    virtual UniquePtr<Message> handle(const NotifyWindowStatusChangedMessage& msg) override;
    virtual UniquePtr<Message> handle(const NotifyWindowIconChangedMessage& msg) override;

private:
    LFoundation::EventLoop& m_event_loop;
};

}