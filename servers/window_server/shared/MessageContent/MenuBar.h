#pragma once
#include <cstdint>
#include <libg/Color.h>

struct StatusBarStyle {
public:
    enum Mode : uint32_t {
        LightText = (1 << 0),
        HideText = (1 << 1),
    };

    StatusBarStyle() = default;
    StatusBarStyle(uint32_t attr)
        : m_flags(attr)
    {
    }

    StatusBarStyle(Mode attr)
        : m_flags(uint32_t(attr))
    {
    }

    StatusBarStyle(uint32_t attr, const LG::Color& clr)
        : m_flags(attr)
        , m_color(clr)
    {
    }

    StatusBarStyle(Mode attr, const LG::Color& clr)
        : m_flags(uint32_t(attr))
        , m_color(clr)
    {
    }

    StatusBarStyle(const LG::Color& clr)
        : m_flags(0)
        , m_color(clr)
    {
    }

    struct StandardLightType {
    };
    static const StandardLightType StandardLight;
    StatusBarStyle(StandardLightType)
        : m_flags(0)
        , m_color(LG::Color::LightSystemBackground)
    {
    }

    struct StandardOpaqueType {
    };
    static const StandardOpaqueType StandardOpaque;
    StatusBarStyle(StandardOpaqueType)
        : m_flags(0)
        , m_color(LG::Color::Opaque)
    {
    }

    ~StatusBarStyle() = default;

    inline bool hide_text() const { return has_attr(Mode::HideText); }
    inline bool show_text() const { return !hide_text(); }

    inline bool light_text() const { return has_attr(Mode::LightText); }
    inline bool dark_text() const { return !light_text(); }

    inline StatusBarStyle& set_light_text()
    {
        set_attr(Mode::LightText);
        return *this;
    }

    inline StatusBarStyle& set_dark_text()
    {
        rem_attr(Mode::LightText);
        return *this;
    }

    inline StatusBarStyle& set_hide_text()
    {
        set_attr(Mode::HideText);
        return *this;
    }

    inline StatusBarStyle& set_show_text()
    {
        rem_attr(Mode::HideText);
        return *this;
    }

    inline uint32_t flags() const { return m_flags; }
    inline void set_flags(uint32_t attr) { m_flags = attr; }

    inline StatusBarStyle& set_mode(Mode attr)
    {
        m_flags = (uint32_t)attr;
        return *this;
    }

    inline const LG::Color& color() const { return m_color; }
    inline void set_color(const LG::Color& clr) { m_color = clr; }

private:
    inline bool has_attr(Mode mode) const { return (m_flags & (uint32_t)mode) == (uint32_t)mode; }
    inline void set_attr(Mode mode) { m_flags |= (uint32_t)mode; }
    inline void rem_attr(Mode mode) { m_flags = m_flags & ~(uint32_t)mode; }

    uint32_t m_flags { 0 };
    LG::Color m_color { LG::Color::LightSystemBackground };
};