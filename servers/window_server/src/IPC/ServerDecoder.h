/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../../shared/Connections/WSConnection.h"
#include "../Managers/Compositor.h"

namespace WinServer {

class WindowServerDecoder : public BaseWindowServerDecoder {
public:
    WindowServerDecoder() = default;
    ~WindowServerDecoder() = default;

    using BaseWindowServerDecoder::handle;
    virtual std::unique_ptr<Message> handle(GreetMessage& msg) override;
    virtual std::unique_ptr<Message> handle(CreateWindowMessage& msg) override;
    virtual std::unique_ptr<Message> handle(DestroyWindowMessage& msg) override;
    virtual std::unique_ptr<Message> handle(SetBarStyleMessage& msg) override;
    virtual std::unique_ptr<Message> handle(SetTitleMessage& msg) override;
    virtual std::unique_ptr<Message> handle(SetBufferMessage& msg) override;
    virtual std::unique_ptr<Message> handle(InvalidateMessage& msg) override;
    virtual std::unique_ptr<Message> handle(MenuBarCreateMenuMessage& msg) override;
    virtual std::unique_ptr<Message> handle(MenuBarCreateItemMessage& msg) override;
    virtual std::unique_ptr<Message> handle(PopupShowMenuMessage& msg) override;
    virtual std::unique_ptr<Message> handle(AskBringToFrontMessage& msg) override;
};

} // namespace WinServer