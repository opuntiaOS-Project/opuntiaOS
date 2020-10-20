/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once

#include "Point.h"
#include <libipc/Decodable.h>
#include <libipc/Encodable.h>
#include <libipc/Encoder.h>
#include <std/String.h>
#include <sys/types.h>
#include <syscalls.h>

namespace LG {

class String : public ::String, public Encodable<String>, public Decodable<String> {
public:
    using ::String::String;

    void encode(EncodedMessage& buf) const override
    {
        for (int i = 0; i < size(); i++) {
            buf.push_back(at(i));
        }
        buf.push_back('\0');
    }

    void decode(const char* buf, size_t& offset) override
    {
        while (buf[offset] != '\0') {
            push_back(buf[offset]);
            offset++;
        }
        offset++;
    }
};
}