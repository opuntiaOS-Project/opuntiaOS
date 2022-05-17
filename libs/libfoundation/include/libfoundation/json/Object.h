/*
 * Copyright (C) 2020-2022 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once

#include <cassert>
#include <cstddef>
#include <map>
#include <string>
#include <vector>

namespace LFoundation::Json {

class Object {
public:
    enum Type {
        Array,
        String,
        Number,
        Dict,
        List,
        Null,
        Bool,
        Invalid,
    };
    constexpr static Object::Type ObjType = Type::Invalid;

    Object() = default;
    Object(Type type)
        : m_type(type)
    {
    }

    virtual ~Object() = default;

    Type type() const { return m_type; }
    bool invalid() const { return type() == Type::Invalid; }

    template <class ObjectT>
    Object* assert_object()
    {
        assert(ObjectT::ObjType == m_type);
        return this;
    }

    template <class ObjectT>
    ObjectT* cast_to_no_assert() { return (ObjectT*)this; }

    template <class ObjectT>
    ObjectT* cast_to() { return (ObjectT*)assert_object<ObjectT>(); }

private:
    Type m_type { Invalid };
};

class StringObject : public Object {
public:
    constexpr static Object::Type ObjType = Type::String;

    StringObject()
        : Object(ObjType)
    {
    }

    ~StringObject() = default;

    std::string& data() { return m_data; }
    const std::string& data() const { return m_data; }

private:
    std::string m_data;
};

class DictObject : public Object {
public:
    constexpr static Object::Type ObjType = Type::Dict;

    DictObject()
        : Object(ObjType)
    {
    }

    ~DictObject()
    {
        // TODO: Add iterators to map.
        // for (auto it : m_data) {
        // delete it.second;
        // }
    }

    std::map<std::string, Object*>& data() { return m_data; }
    const std::map<std::string, Object*>& data() const { return m_data; }

private:
    std::map<std::string, Object*> m_data;
};

class ListObject : public Object {
public:
    constexpr static Object::Type ObjType = Type::List;

    ListObject()
        : Object(ObjType)
    {
    }

    ~ListObject()
    {
        for (auto& i : m_data) {
            delete i;
        }
    }

    std::vector<Object*>& data() { return m_data; }
    const std::vector<Object*>& data() const { return m_data; }

private:
    std::vector<Object*> m_data;
};

class NullObject : public Object {
public:
    constexpr static Object::Type ObjType = Type::Null;

    NullObject()
        : Object(ObjType)
    {
    }

    ~NullObject() = default;

private:
};

class BoolObject : public Object {
public:
    constexpr static Object::Type ObjType = Type::Bool;

    BoolObject()
        : Object(ObjType)
    {
    }

    ~BoolObject() = default;

    bool& data() { return m_data; }
    const bool& data() const { return m_data; }

private:
    bool m_data;
};

class InvalidObject : public Object {
public:
    constexpr static Object::Type ObjType = Type::Invalid;

    InvalidObject()
        : Object(ObjType)
    {
    }

    ~InvalidObject() = default;
};

} // namespace LFoundation