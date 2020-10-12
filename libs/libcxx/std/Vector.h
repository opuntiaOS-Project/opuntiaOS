#pragma once
#include <libcxx/malloc.h>
#include <libcxx/memory.h>

namespace Algo {

template <class T>
class Vector {
public:
    typedef T* iterator;

    Vector()
    {
        grow(m_capacity);
    }

    Vector(int capacity)
    {
        grow(capacity);
    }

    Vector(const Vector& v)
    {
        grow(v.m_capacity);
        m_size = v.m_size;
        copy(m_data, v.m_data, m_size);
    }

    Vector(Vector&& v)
    {
        m_size = v.m_size;
        m_capacity = v.m_capacity;
        m_data = v.m_data;
        v.m_size = v.m_capacity = 0;
        v.m_data = nullptr;
    }

    ~Vector()
    {
        clear();
    }

    Vector& operator=(const Vector& v)
    {
        grow(v.m_capacity);
        m_size = v.m_size;
        copy(m_data, v.m_data, m_size);
        return *this;
    }

    Vector& operator=(Vector&& v)
    {
        if (this != &v) {
            clear();
            m_size = v.m_size;
            m_capacity = v.m_capacity;
            m_data = v.m_data;
            v.m_size = 0;
            v.m_capacity = 0;
            v.m_data = nullptr;
        }
        return *this;
    }

    inline void push_back(T&& el)
    {
        ensure_capacity(size() + 1);
        new (end()) T(move(el));
        m_size++;
    }

    inline void push_back(const T& el)
    {
        push_back(T(el));
    }

    inline const T& at(size_t i) const
    {
        return data()[i];
    }

    inline T& at(size_t i)
    {
        return data()[i];
    }

    void clear()
    {
        clear_remain_capacity();
        if (m_data) {
            free(m_data);
            m_data = nullptr;
        }
    }

    void clear_remain_capacity()
    {
        for (size_t i = 0; i < m_size; ++i) {
            data()[i].~T();
        }
        m_size = 0;
    }

    inline size_t size() const { return m_size; }

    inline const T& operator[](size_t i) const { return at(i); }
    inline T& operator[](size_t i) { return at(i); }

    inline const T& back() const { return at(size() - 1); }
    inline T& back() { return at(size() - 1); }

    inline T* data() { return m_data; }
    inline const T* data() const { return m_data; }

private:
    inline void ensure_capacity(size_t new_size)
    {
        if (m_capacity == 0) {
            grow(16);
        } else if (new_size > m_capacity) {
            grow(m_capacity * 2);
        }
    }

    void grow(size_t capacity)
    {
        if (!m_data) {
            m_data = (T*)malloc(capacity * sizeof(T));
        } else {
            auto new_buf = (T*)malloc(capacity * sizeof(T));

            for (size_t i = 0; i < m_size; i++) {
                new (&new_buf[i]) T(move(at(i)));
                at(i).~T();
            }
            free(m_data);
            m_data = new_buf;
        }
        m_capacity = capacity;
    }

    void copy(T* to, T* from, size_t len)
    {
        for (size_t i = 0; i < len; i++) {
            new (to) T(*from);
            to++;
            from++;
        }
    }

    inline T* end() { return m_data + m_size; }

    size_t m_size { 0 };
    size_t m_capacity { 16 };
    T* m_data { nullptr };
};
}

using Algo::Vector;