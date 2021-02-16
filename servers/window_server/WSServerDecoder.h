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
    WServerDecoder() { }
    ~WServerDecoder() { }

    using WindowServerDecoder::handle;
    virtual UniquePtr<Message> handle(const GreetMessage& msg) override;
    virtual UniquePtr<Message> handle(const CreateWindowMessage& msg) override;
    virtual UniquePtr<Message> handle(const DestroyWindowMessage& msg) override;
    virtual UniquePtr<Message> handle(const SetBarStyleMessage& msg) override;
    virtual UniquePtr<Message> handle(const SetTitleMessage& msg) override;
    virtual UniquePtr<Message> handle(const SetBufferMessage& msg) override;
    virtual UniquePtr<Message> handle(const InvalidateMessage& msg) override;
    virtual UniquePtr<Message> handle(const AskBringToFrontMessage& msg) override;
};
