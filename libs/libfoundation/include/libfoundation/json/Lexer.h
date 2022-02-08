/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <cctype>
#include <string>
#include <unistd.h>

namespace LFoundation::Json {

class Lexer {
public:
    Lexer() = default;
    Lexer(const std::string& filepath)
        : m_text_data()
        , m_pointer(0)
    {
        set_file(filepath);
    }

    ~Lexer() = default;

    int set_file(const std::string& filepath);

    bool is_eof() const { return m_pointer >= m_text_data.size(); }

    int lookup_char() const
    {
        if (is_eof()) {
            return EOF;
        }
        return m_text_data[m_pointer];
    }

    int next_char()
    {
        if (is_eof()) {
            return EOF;
        }
        return m_text_data[m_pointer++];
    }

    void skip_spaces()
    {
        while (std::isspace(lookup_char())) {
            next_char();
        }
    }

    bool eat_string(std::string& word)
    {
        // TODO: Fix stop at "
        while (std::isprint(lookup_char()) && lookup_char() != '\"') {
            word.push_back(next_char());
        };
        return true;
    }

    bool eat_token(char what)
    {
        skip_spaces();
        return next_char() == what;
    }

private:
    int m_pointer;
    std::string m_text_data;
};

} // namespace LFoundation