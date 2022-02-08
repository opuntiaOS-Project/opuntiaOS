/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include <cassert>
#include <libfoundation/json/Parser.h>
#include <unistd.h>

namespace LFoundation::Json {

StringObject* Parser::parse_string()
{
    auto* res = new StringObject();

    m_lexer.skip_spaces();
    assert(m_lexer.eat_token('\"'));
    m_lexer.eat_string(res->data());
    assert(m_lexer.eat_token('\"'));
    return res;
}

DictObject* Parser::parse_dict()
{
    auto* res = new DictObject();

    m_lexer.skip_spaces();
    assert(m_lexer.eat_token('{'));
    for (;;) {
        auto left = parse_string();
        assert(m_lexer.eat_token(':'));
        auto right_obj = parse_object();
        res->data()[left->data()] = right_obj;

        if (m_lexer.lookup_char() == ',') {
            assert(m_lexer.eat_token(','));
        } else {
            break;
        }
    }
    assert(m_lexer.eat_token('}'));
    return res;
}

Object* Parser::parse_object()
{
    m_lexer.skip_spaces();
    switch (m_lexer.lookup_char()) {
    case '{':
        return parse_dict();
    case '\"':
        return parse_string();
    default:
        assert(false);
    }
    return new DictObject();
}

}