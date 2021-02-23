#pragma once

#include <std/Utility.h>
#include <syscalls.h>

template <typename T>
class UniquePtr {
public:
    UniquePtr()
        : m_data(nullptr)
    {
    }

    UniquePtr(T* data)
        : m_data(data)
    {
    }

    UniquePtr(nullptr_t)
        : m_data(nullptr)
    {
    }

    UniquePtr& operator=(nullptr_t)
    {
        reset();
        return *this;
    }

    UniquePtr(UniquePtr&& moving) noexcept
        : m_data(nullptr)
    {
        moving.swap(*this);
    }

    UniquePtr& operator=(UniquePtr&& moving) noexcept
    {
        moving.swap(*this);
        return *this;
    }

    template <typename U>
    UniquePtr(UniquePtr<U>&& moving)
    {
        UniquePtr<T> tmp((T*)moving.release());
        tmp.swap(*this);
    }

    template <typename U>
    UniquePtr& operator=(UniquePtr<U>&& moving)
    {
        UniquePtr<T> tmp((T*)moving.release());
        tmp.swap(*this);
        return *this;
    }

    T* release() noexcept
    {
        T* result = m_data;
        m_data = nullptr;
        return result;
    }

    void swap(UniquePtr& src) noexcept
    {
        T* tmp_data = m_data;
        m_data = src.m_data;
        src.m_data = tmp_data;
    }

    void reset()
    {
        T* tmp = release();
        delete tmp;
    }

    ~UniquePtr()
    {
        delete m_data;
    }

    UniquePtr(UniquePtr const&) = delete;
    UniquePtr& operator=(UniquePtr const&) = delete;

    T* operator->() const { return m_data; }
    T& operator*() const { return *m_data; }

    T* get() const { return m_data; }
    explicit operator bool() const { return m_data; }

private:
    T* m_data { nullptr };
};