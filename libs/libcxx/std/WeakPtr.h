#pragma once

#include <libcxx/std/Utility.h>
#include <libcxx/syscalls.h>

template <typename T>
class Weakable;

template <typename T>
class WeakLink {
public:
    friend class Weakable<T>;

    WeakLink(T& data)
        : m_ptr(&data)
    {
    }

    operator bool() const { return ptr(); }

    T* ptr() { return m_ptr; }
    const T* ptr() const { return m_ptr; }

private:
    T* m_ptr { nullptr };
};

template <typename T>
class WeakPtr {
public:
    friend class Weakable<T>;

    WeakPtr()
        : m_data(nullptr)
    {
    }

    WeakPtr& operator=(nullptr_t)
    {
        m_data = nullptr;
        return *this;
    }

    operator bool() const { return ptr(); }

    T* ptr() { return m_data ? m_data->ptr() : nullptr; }
    const T* ptr() const { return m_data ? m_data->ptr() : nullptr; }

    T* operator->() { return ptr(); }
    const T* operator->() const { return ptr(); }

    T& operator*() { return *ptr(); }
    const T& operator*() const { return *ptr(); }

    void clear() { m_data = nullptr; }

private:
    WeakPtr(WeakLink<T>* data)
        : m_data(data)
    {
    }

    WeakLink<T>* m_data { nullptr };
};

template <typename T>
class Weakable {
public:
    WeakPtr<T> weak_ptr()
    {
        if (!m_data) {
            m_data = new WeakLink<T>(static_cast<T&>(*this));
        }
        return WeakPtr<T>(m_data);
    }

protected:
    Weakable() { }

    ~Weakable()
    {
        if (m_data) {
            m_data->m_ptr = nullptr;
        }
    }

private:
    WeakLink<T>* m_data { nullptr };
};