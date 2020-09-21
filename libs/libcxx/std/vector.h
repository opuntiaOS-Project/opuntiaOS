#pragma once
#include <libcxx/malloc.h>
#include <libcxx/memory.h>

namespace Algo {

template <typename T>
class Vector {
public:
    Vector()
    {
        grow(m_capacity);
    }

    Vector(int capacity)
    {
        grow(capacity);
    }

    ~Vector()
    {
        clear();
    }

    inline void push_back(T&& el)
    {
        ensure_capacity(size() + 1);
        m_data[m_size] = move(el);
        m_size++;
    }

    inline void push_back(const T& el)
    {
        push_back(T(el));
    }

    inline const T& at(size_t i) const
    {
        // ASSERT(i < m_size);
        return data()[i];
    }
    inline T& at(size_t i)
    {
        // ASSERT(i < m_size);
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

private:
    inline void ensure_capacity(size_t new_size)
    {
        if (new_size > m_capacity) {
            grow(m_capacity * 2);
        }
    }

    void grow(size_t capacity)
    {
        T* old_ptr = m_data;
        if (!old_ptr) {
            m_data = (T*)malloc(capacity * sizeof(T));
        } else {
            m_data = (T*)realloc((void*)old_ptr, capacity * sizeof(T));
        }
        m_capacity = capacity;
    }

    size_t m_size { 0 };
    size_t m_capacity { 16 };
    T* m_data { nullptr };
};
}

using Algo::Vector;