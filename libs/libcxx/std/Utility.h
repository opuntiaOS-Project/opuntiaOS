#pragma once

template <typename T>
inline T&& move(T& arg)
{
    return static_cast<T&&>(arg);
}

template <typename T, typename U>
inline void swap(T& a, U& b)
{
    U tmp = move((U&)a);
    a = (T &&) move(b);
    b = move(tmp);
}
