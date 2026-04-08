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

  struct MarginLeft
  {
    MarginLeft() = default;

    constexpr explicit MarginLeft(int m = 0)
        : margin { m }
    {
    }

    int margin;
    bool operator==(const MarginLeft &) const = default;
  };

  struct MarginRight
  {
    MarginRight() = default;

    constexpr explicit MarginRight(int m = 0)
        : margin { m }
    {
    }

    int margin;
    bool operator==(const MarginRight &) const = default;
  };

  struct MarginTop
  {
    MarginTop() = default;

    constexpr explicit MarginTop(int m = 0)
        : margin { m }
    {
    }

    int margin;
    bool operator==(const MarginTop &) const = default;
  };

  struct MarginBottom
  {
    MarginBottom() = default;

    constexpr explicit MarginBottom(int m = 0)
        : margin { m }
    {
    }

    int margin;
    bool operator==(const MarginBottom &) const = default;
  };
}