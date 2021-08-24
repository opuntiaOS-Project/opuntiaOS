/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../shared/Connections/WSConnection.h"
#include "Compositor.h"

namespace WinServer {

class WindowServerDecoder : public BaseWindowServerDecoder {
public:
    WindowServerDecoder() = default;
    ~WindowServerDecoder() = default;

    using BaseWindowServerDecoder::handle;
    virtual std::unique_ptr<Message> handle(const GreetMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const CreateWindowMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const DestroyWindowMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const SetBarStyleMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const SetTitleMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const SetBufferMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const InvalidateMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const MenuBarCreateMenuMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const MenuBarCreateItemMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const AskBringToFrontMessage& msg) override;
};

} // namespace WinServer