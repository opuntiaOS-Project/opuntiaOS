/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <cstddef>

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

} // namespace LFoundation