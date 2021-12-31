/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
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
    virtual std::unique_ptr<Message> handle(MouseMoveMessage& msg) override;
    virtual std::unique_ptr<Message> handle(MouseActionMessage& msg) override;
    virtual std::unique_ptr<Message> handle(MouseLeaveMessage& msg) override;
    virtual std::unique_ptr<Message> handle(MouseWheelMessage& msg) override;
    virtual std::unique_ptr<Message> handle(KeyboardMessage& msg) override;
    virtual std::unique_ptr<Message> handle(DisplayMessage& msg) override;
    virtual std::unique_ptr<Message> handle(WindowCloseRequestMessage& msg) override;
    virtual std::unique_ptr<Message> handle(ResizeMessage& msg) override;
    virtual std::unique_ptr<Message> handle(MenuBarActionMessage& msg) override;
    virtual std::unique_ptr<Message> handle(PopupActionMessage& msg) override;

    // Notifiers
    virtual std::unique_ptr<Message> handle(NotifyWindowCreateMessage& msg) override;
    virtual std::unique_ptr<Message> handle(NotifyWindowStatusChangedMessage& msg) override;
    virtual std::unique_ptr<Message> handle(NotifyWindowIconChangedMessage& msg) override;
    virtual std::unique_ptr<Message> handle(NotifyWindowTitleChangedMessage& msg) override;

private:
    LFoundation::EventLoop& m_event_loop;
};

} // namespace UI