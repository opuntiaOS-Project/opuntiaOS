/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
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
    virtual UniquePtr<Message> handle(const SetTitleMessage& msg) override;
    virtual UniquePtr<Message> handle(const SetBufferMessage& msg) override;
    virtual UniquePtr<Message> handle(const InvalidateMessage& msg) override;
};
