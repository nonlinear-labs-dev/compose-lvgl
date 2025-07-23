#pragma once
#include <string>
#include <format>

namespace Compose
{
  struct Padding
  {
    static constexpr auto key = "_Padding";

    Padding() = default;
    constexpr explicit Padding(int m)
        : left { m }
        , right { m }
        , top { m }
        , bottom { m }
    {
    }

    constexpr Padding(int t, int b, int l, int r)
        : left(l)
        , right(r)
        , top(t)
        , bottom(b)
    {
    }

    constexpr static Padding ZERO()
    {
      return { 0, 0, 0, 0 };
    }

    int left;
    int right;
    int top;
    int bottom;
  };
}
