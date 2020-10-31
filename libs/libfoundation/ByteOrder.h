/*
 * Copyright (C) 2020 Nikita Melekhin
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License v2 as published by the
 * Free Software Foundation.
 */

#pragma once

namespace LFoundation {

class ByteOrder {
public:
    template <typename T>
    [[gnu::always_inline]] static inline T from_network(T value)
    {
        if constexpr (sizeof(T) == 8) {
            return __builtin_bswap64(value);
        }
        if constexpr (sizeof(T) == 4) {
            return __builtin_bswap32(value);
        }
        if constexpr (sizeof(T) == 2) {
            return __builtin_bswap16(value);
        }
        if constexpr (sizeof(T) == 1) {
            return value;
        }
    }
};

}