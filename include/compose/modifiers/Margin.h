#pragma once

namespace Compose
{
  struct Margin
  {
    Margin() = default;
    constexpr explicit Margin(int m)
        : top { m }
        , bottom { m }
        , left { m }
        , right { m }
    {
    }

    constexpr Margin(int t, int b, int l, int r)
        : top(t)
        , bottom(b)
        , left(l)
        , right(r)
    {
    }

    template <typename T>
    explicit constexpr Margin(T o)
        : top(o.top)
        , bottom(o.bottom)
        , left(o.left)
        , right(o.right)
    {
    }

    constexpr static Margin ZERO()
    {
      return { 0, 0, 0, 0 };
    }

    int top, bottom, left, right;

    bool operator==(const Margin &) const = default;
  };
}