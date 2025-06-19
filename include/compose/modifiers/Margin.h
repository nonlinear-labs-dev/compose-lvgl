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

    constexpr static Margin ZERO()
    {
      return { 0, 0, 0, 0 };
    }

    int top, bottom, left, right;
  };
}