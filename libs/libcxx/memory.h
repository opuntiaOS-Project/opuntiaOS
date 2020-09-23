#pragma once

template <typename T>
inline T&& move(T& arg)
{
    return static_cast<T&&>(arg);
}

template <typename T>
class unique_ptr {
public:
    unique_ptr()
        : m_data(nullptr)
    {
    }

    unique_ptr(T* data)
        : m_data(data)
    {
    }

    unique_ptr(typeof(nullptr))
        : m_data(nullptr)
    {
    }
    unique_ptr& operator=(typeof(nullptr))
    {
        reset();
        return *this;
    }

    unique_ptr(unique_ptr&& moving) noexcept
    {
        moving.swap(*this);
    }
    unique_ptr& operator=(unique_ptr&& moving) noexcept
    {
        moving.swap(*this);
        return *this;
    }

    template <typename U>
    unique_ptr(unique_ptr<U>&& moving)
    {
        unique_ptr<T> tmp(moving.release());
        tmp.swap(*this);
    }
    template <typename U>
    unique_ptr& operator=(unique_ptr<U>&& moving)
    {
        unique_ptr<T> tmp(moving.release());
        tmp.swap(*this);
        return *this;
    }

    T* release() noexcept
    {
        T* result = m_data;
        m_data = nullptr;
        return result;
    }

    void swap(unique_ptr& src) noexcept
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

    ~unique_ptr()
    {
        delete m_data;
    }

    unique_ptr(unique_ptr const&) = delete;
    unique_ptr& operator=(unique_ptr const&) = delete;

    T* operator->() const { return m_data; }
    T& operator*() const { return *m_data; }

    T* get() const { return m_data; }
    explicit operator bool() const { return m_data; }

private:
    T* m_data;
};