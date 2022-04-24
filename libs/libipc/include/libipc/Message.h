/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include <sys/types.h>
#include <vector>

typedef std::vector<uint8_t> EncodedMessage;
typedef int message_key_t;

class Message {
public:
    Message() = default;
    virtual ~Message() = default;

    virtual int decoder_magic() const { return 0; }
    virtual int id() const { return 0; }
    virtual message_key_t key() const { return -1; }
    virtual int reply_id() const { return -1; } // -1 means that there is no reply.
    virtual EncodedMessage encode() const { return std::vector<uint8_t>(); }
};