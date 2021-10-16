/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <iostream>
#include <libipc/Decodable.h>
#include <libipc/Encodable.h>
#include <libipc/Encoder.h>
#include <sys/types.h>
#include <vector>

namespace LIPC {

template <class T>
class VectorEncoder : public Encodable<VectorEncoder<T>>, public Decodable<VectorEncoder<T>> {
public:
    VectorEncoder() = default;
    VectorEncoder(const std::vector<T>& vec)
        : m_vec(vec)
    {
    }

    VectorEncoder(std::vector<T>&& vec)
        : m_vec(std::move(vec))
    {
    }

    ~VectorEncoder() = default;

    void encode(EncodedMessage& buf) const override
    {
        size_t sz = m_vec.size();
        Encoder::append(buf, sz);
        for (int i = 0; i < sz; i++) {
            Encoder::append(buf, m_vec[i]);
        }
    }

    void decode(const char* buf, size_t& offset) override
    {
        size_t sz = 0;
        Encoder::decode(buf, offset, sz);
        for (int i = 0; i < sz; i++) {
            T el;
            Encoder::decode(buf, offset, el);
            m_vec.push_back(std::move(el));
        }
    }

    std::vector<T>& vector() { return m_vec; }
    const std::vector<T>& vector() const { return m_vec; }
    std::vector<T>&& move_vector() { return std::move(m_vec); }

private:
    std::vector<T> m_vec {};
};
} // namespace LIPC