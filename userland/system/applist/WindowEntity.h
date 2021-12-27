#pragma once
#include <libg/PixelBitmap.h>
#include <string>

class WindowEntity {
public:
    enum Status : uint32_t {
        Minimized = (1 << 0),
    };

    WindowEntity()
        : m_window_id(0)
        , m_title()
    {
    }

    WindowEntity(int window_id)
        : m_window_id(window_id)
        , m_title()
    {
    }

    bool operator==(const WindowEntity& other) const { return m_window_id == other.m_window_id; }
    bool operator!=(const WindowEntity& other) const { return m_window_id != other.m_window_id; }

    int window_id() const { return m_window_id; }

    bool is_minimized() const { return has_attr(Status::Minimized); }
    void set_minimized(bool b) { b ? set_attr(Status::Minimized) : rem_attr(Status::Minimized); }

    const std::string& title() const { return m_title; }
    void set_title(const std::string& title) { m_title = title; }
    void set_title(std::string&& title) { m_title = std::move(title); }

private:
    inline bool has_attr(Status mode) const { return ((m_window_status & (uint32_t)mode) == (uint32_t)mode); }
    inline void set_attr(Status mode) { m_window_status |= (uint32_t)mode; }
    inline void rem_attr(Status mode) { m_window_status = m_window_status & (~(uint32_t)mode); }

    int m_window_id { 0 };
    uint32_t m_window_status { 0 };
    std::string m_title {};
};