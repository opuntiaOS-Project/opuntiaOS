/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <algo/hash.h>

uint32_t hash_crc32(uint8_t* data, size_t len)
{
    uint32_t byte, mask;
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < len; i++) {
        byte = data[i];
        crc ^= byte;
        for (int j = 0; j < 8; j++) {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    }
    return ~crc;
}

uint32_t hashstr_crc32(char* data)
{
    uint32_t byte, mask;
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; data[i]; i++) {
        byte = data[i];
        crc ^= byte;
        for (int j = 0; j < 8; j++) {
            mask = -(crc & 1);
            crc = (crc >> 1) ^ (0xEDB88320 & mask);
        }
    }
    return ~crc;
}