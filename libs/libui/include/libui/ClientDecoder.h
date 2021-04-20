/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../../../servers/window_server/shared/Connections/WSConnection.h"
#include <libfoundation/EventLoop.h>

namespace UI {

class App;

class ClientDecoder : public BaseWindowClientDecoder {
public:
    ClientDecoder();
    ~ClientDecoder() = default;

    using BaseWindowClientDecoder::handle;
    virtual std::unique_ptr<Message> handle(const MouseMoveMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const MouseActionMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const MouseLeaveMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const KeyboardMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const DisplayMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const WindowCloseRequestMessage& msg) override;

    // Notifiers
    virtual std::unique_ptr<Message> handle(const NotifyWindowStatusChangedMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const NotifyWindowIconChangedMessage& msg) override;

private:
    LFoundation::EventLoop& m_event_loop;
};

} // namespace UI