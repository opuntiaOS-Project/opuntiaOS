/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
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