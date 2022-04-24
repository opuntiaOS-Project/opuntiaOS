/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <libipc/Message.h>
#include <memory>

class MessageDecoder {
public:
    MessageDecoder() = default;
    virtual ~MessageDecoder() = default;

    virtual int magic() { return 0; }
    virtual std::unique_ptr<Message> decode(const char* buf, size_t size, size_t& decoded_msg_len) { return nullptr; }
    virtual std::unique_ptr<Message> handle(Message&) { return nullptr; }
};