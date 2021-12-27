/*
 * Copyright (C) 2020-2021 The opuntiaOS Project Authors.
 *  + Contributed by Nikita Melekhin <nimelehin@gmail.com>
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#pragma once
#include "../Devices/Screen.h"
#include "../IPC/Event.h"
#include <algorithm>
#include <libg/PixelBitmap.h>
#include <libg/Point.h>

#ifdef TARGET_DESKTOP
#define CURSOR_OFFSET (2)
#elif TARGET_MOBILE
#define CURSOR_OFFSET (6)
#endif

namespace WinServer {

class CursorManager {
public:
    enum class Params {
        X,
        Y,
        LeftButton,
        RightButton,
        OffsetX,
        OffsetY,
        Wheel,

        // Only for getters of conditions changes.
        Coords,
        Buttons,
    };

    inline static CursorManager& the()
    {
        extern CursorManager* s_WinServer_CursorManager_the;
        return *s_WinServer_CursorManager_the;
    }

    CursorManager();
    ~CursorManager() = default;

    inline const LG::PixelBitmap& current_cursor() const { return std_cursor(); }
    inline const LG::PixelBitmap& std_cursor() const { return m_std_cursor; }
    inline LG::Point<int> draw_position() { return { m_mouse_x - CURSOR_OFFSET, m_mouse_y - CURSOR_OFFSET }; }

    inline int x() const
    {
        return m_mouse_x;
    }
    inline int y() const { return m_mouse_y; }

    template <Params param>
    constexpr int get();

    template <Params param>
    constexpr bool pressed() const;

    template <Params param>
    constexpr bool is_changed();
    inline void clear_changed(uint32_t val = 0) { m_mask_changed_objects = val; }

    template <Params param, typename Value>
    inline constexpr void set(Value val)
    {
        if constexpr (param == CursorManager::Params::X) {
            val = std::max(std::min(val, (int)m_screen.width() - 1), 0);
            m_mouse_offset_x = val - m_mouse_x;
            m_mouse_x = val;
            set_changed<CursorManager::Params::X>();
        } else if constexpr (param == CursorManager::Params::Y) {
            val = std::max(std::min(val, (int)m_screen.height() - 1), 0);
            m_mouse_offset_y = val - m_mouse_y;
            m_mouse_y = val;
            set_changed<CursorManager::Params::Y>();
        } else if constexpr (param == CursorManager::Params::OffsetX) {
            if (val != 0) {
                set<CursorManager::Params::X>(m_mouse_x + val);
            } else {
                m_mouse_offset_x = 0;
            }
        } else if constexpr (param == CursorManager::Params::OffsetY) {
            if (val != 0) {
                set<CursorManager::Params::Y>(m_mouse_y + val);
            } else {
                m_mouse_offset_y = 0;
            }
        } else if constexpr (param == CursorManager::Params::LeftButton) {
            if (m_mouse_left_button_pressed != val) {
                set_changed<CursorManager::Params::LeftButton>();
            }
            m_mouse_left_button_pressed = val;
        } else if constexpr (param == CursorManager::Params::RightButton) {
            if (m_mouse_right_button_pressed != val) {
                set_changed<CursorManager::Params::RightButton>();
            }
            m_mouse_right_button_pressed = val;
        } else if constexpr (param == CursorManager::Params::Wheel) {
            if (val != 0) {
                set_changed<CursorManager::Params::Wheel>();
            }
            m_wheel = val;
        } else {
            []<bool flag = false>() { static_assert(flag, "Could not call set() with such param!"); }
            ();
        }
    }

    template <Params param>
    inline constexpr void set_changed()
    {
        if constexpr (param == CursorManager::Params::X) {
            m_mask_changed_objects |= CursorManager::ChangedValues::MouseCoords;
        } else if constexpr (param == CursorManager::Params::Y) {
            m_mask_changed_objects |= CursorManager::ChangedValues::MouseCoords;
        } else if constexpr (param == CursorManager::Params::LeftButton) {
            m_mask_changed_objects |= CursorManager::ChangedValues::LeftButton;
        } else if constexpr (param == CursorManager::Params::RightButton) {
            m_mask_changed_objects |= CursorManager::ChangedValues::RightButton;
        } else if constexpr (param == CursorManager::Params::Wheel) {
            m_mask_changed_objects |= CursorManager::ChangedValues::Wheel;
        } else {
            []<bool flag = false>() { static_assert(flag, "Could not set_changed() for the param!"); }
            ();
        }
    }

    void update_position(MouseEvent* mouse_event)
    {
        clear_changed();
        set<Params::OffsetX>(mouse_event->packet().x_offset);
        set<Params::OffsetY>(-mouse_event->packet().y_offset);
        set<Params::LeftButton>((mouse_event->packet().button_states & 1));
        set<Params::RightButton>((mouse_event->packet().button_states & 2) >> 1);
        set<Params::Wheel>(mouse_event->packet().wheel_data);
    }

private:
    enum ChangedValues {
        MouseCoords = 0x1,
        LeftButton = 0x2,
        RightButton = 0x4,
        Wheel = 0x8,
    };

    int m_mouse_x { 0 };
    int m_mouse_y { 0 };
    int m_mouse_offset_x { 0 };
    int m_mouse_offset_y { 0 };
    int m_wheel { 0 };
    bool m_mouse_left_button_pressed { false };
    bool m_mouse_right_button_pressed { false };
    uint32_t m_mask_changed_objects { 0 };
    bool m_mouse_changed_button_status { false };

    Screen& m_screen;
    LG::PixelBitmap m_std_cursor;
};

template <CursorManager::Params param>
inline constexpr int CursorManager::get()
{
    if constexpr (param == CursorManager::Params::X) {
        return m_mouse_x;
    } else if constexpr (param == CursorManager::Params::Y) {
        return m_mouse_y;
    } else if constexpr (param == CursorManager::Params::OffsetX) {
        return m_mouse_offset_x;
    } else if constexpr (param == CursorManager::Params::OffsetY) {
        return m_mouse_offset_y;
    } else if constexpr (param == CursorManager::Params::Wheel) {
        return m_wheel;
    } else {
        []<bool flag = false>() { static_assert(flag, "Could call get() only for coords-like params!"); }
        ();
    }
}

template <CursorManager::Params param>
inline constexpr bool CursorManager::pressed() const
{
    if constexpr (param == CursorManager::Params::LeftButton) {
        return m_mouse_left_button_pressed;
    } else if constexpr (param == CursorManager::Params::RightButton) {
        return m_mouse_right_button_pressed;
    } else {
        []<bool flag = false>() { static_assert(flag, "Could call pressed() only for buttons!"); }
        ();
    }
}

template <CursorManager::Params param>
inline constexpr bool CursorManager::is_changed()
{
    if constexpr (param == CursorManager::Params::X) {
        return (m_mask_changed_objects & CursorManager::ChangedValues::MouseCoords);
    } else if constexpr (param == CursorManager::Params::Y) {
        return (m_mask_changed_objects & CursorManager::ChangedValues::MouseCoords);
    } else if constexpr (param == CursorManager::Params::LeftButton) {
        return (m_mask_changed_objects & CursorManager::ChangedValues::LeftButton);
    } else if constexpr (param == CursorManager::Params::RightButton) {
        return (m_mask_changed_objects & CursorManager::ChangedValues::RightButton);
    } else if constexpr (param == CursorManager::Params::Coords) {
        return (m_mask_changed_objects & CursorManager::ChangedValues::MouseCoords);
    } else if constexpr (param == CursorManager::Params::Buttons) {
        return (m_mask_changed_objects & CursorManager::ChangedValues::LeftButton) | (m_mask_changed_objects & CursorManager::ChangedValues::RightButton);
    } else if constexpr (param == CursorManager::Params::Wheel) {
        return (m_mask_changed_objects & CursorManager::ChangedValues::Wheel);
    } else {
        []<bool flag = false>() { static_assert(flag, "Could not call is_changed() for the param!"); }
        ();
    }
}

} // namespace WinServer
