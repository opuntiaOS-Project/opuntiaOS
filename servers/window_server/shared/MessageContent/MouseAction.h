#pragma once
#include <cstddef>

enum MouseActionType {
    LeftMouseButtonPressed,
    LeftMouseButtonReleased,
    RightMouseButtonPressed,
    RightMouseButtonReleased,
};

class MouseActionState {
public:
    MouseActionState() = default;
    ~MouseActionState() = default;

    inline int state() const { return m_state; }
    inline void set(MouseActionType state) { m_state |= (int)state; }

private:
    int m_state { 0 };
};