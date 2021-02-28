/*
 * Copyright (C) 2020-2021 Nikita Melekhin. All rights reserved.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "Compositor.h"

class WServerDecoder : public WindowServerDecoder {
public:
    WServerDecoder() = default;
    ~WServerDecoder() = default;

    using WindowServerDecoder::handle;
    virtual std::unique_ptr<Message> handle(const GreetMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const CreateWindowMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const DestroyWindowMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const SetBarStyleMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const SetTitleMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const SetBufferMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const InvalidateMessage& msg) override;
    virtual std::unique_ptr<Message> handle(const AskBringToFrontMessage& msg) override;
};
