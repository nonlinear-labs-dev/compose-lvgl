#pragma once

namespace Compose
{
  struct Position
  {
    constexpr Position() = default;

    template <typename T>
    constexpr Position(T _x, T _y)
        : x(_x)
        , y(_y)
    {
    }

    template <typename T>
    constexpr explicit Position(T t)
        : x(t.w)
        , y(t.h)
    {
    }

    template <typename T> constexpr Position operator*(T v) const
    {
      return Position { x * v, y * v };
    }

    static constexpr Position ZERO()
    {
      return { 0, 0 };
    }

    int x {};
    int y {};
  };
}
