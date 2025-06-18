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

    constexpr Padding(int l, int r, int t, int b)
        : left(l)
        , right(r)
        , top(t)
        , bottom(b)
    {
    }

    std::string cssRuleText() const
    {
      return std::format("padding: {}px {}px {}px {}px;", top, right, bottom, left);
    }

    int left;
    int right;
    int top;
    int bottom;
  };
}
