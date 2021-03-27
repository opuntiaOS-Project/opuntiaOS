#pragma once
#include <sys/shared_buffer.h>

namespace LFoundation {

template <typename T>
class SharedBuffer {
public:
    SharedBuffer() = default;
    SharedBuffer(size_t size)
        : m_size(size)
    {
        m_id = shared_buffer_create((uint8_t**)&m_data, m_size);
    }

    SharedBuffer(int id)
        : m_id(id)
    {
        if (shared_buffer_get(m_id, (uint8_t**)&m_data) != 0) {
            m_id = -1;
        }
    }

    ~SharedBuffer()
    {
    }

    void create(size_t size)
    {
        m_size = size;
        m_id = shared_buffer_create((uint8_t**)&m_data, m_size);
    }

    void open(int id)
    {
        m_id = id;
        if (shared_buffer_get(m_id, (uint8_t**)&m_data) != 0) {
            m_id = -1;
        }
    }

    void free()
    {
        if (alive()) {
            shared_buffer_free(id());
            m_id = -1;
        }
    }

    inline bool alive() const { return m_id >= 0; }

    inline const T& at(size_t i) const
    {
        return data()[i];
    }

    inline T& at(size_t i)
    {
        return data()[i];
    }

    inline size_t size() const { return m_size; }

    inline const T& operator[](size_t i) const { return at(i); }
    inline T& operator[](size_t i) { return at(i); }

    inline int id() const { return m_id; }
    inline T* data() { return m_data; }

private:
    int m_id { -1 };
    size_t m_size { 0 };
    T* m_data { nullptr };
};
} // namespace LFoundation