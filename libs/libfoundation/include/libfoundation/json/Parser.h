/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <libfoundation/json/Lexer.h>
#include <libfoundation/json/Object.h>
#include <string>

namespace LFoundation::Json {

class Parser {
public:
    Parser(const std::string& filepath)
    {
        int err = m_lexer.set_file(filepath);
        if (err) {
            m_root_object = new InvalidObject();
        }
    }

    ~Parser() = default;

    Object* object()
    {
        if (!m_root_object) {
            m_root_object = parse_object();
        }
        return m_root_object;
    }

private:
    StringObject* parse_string();
    DictObject* parse_dict();
    Object* parse_object();

    Object* m_root_object { nullptr };
    Lexer m_lexer;
};

} // namespace LFoundation