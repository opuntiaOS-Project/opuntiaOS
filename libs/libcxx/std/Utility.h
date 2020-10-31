#pragma once

template <typename T>
inline constexpr T min(const T& a, const T& b)
{
    return a < b ? a : b;
}

template <typename T>
inline constexpr T max(const T& a, const T& b)
{
    return a < b ? b : a;
}

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

inline constexpr int abs(int i)
{
    return i < 0 ? -i : i;
}