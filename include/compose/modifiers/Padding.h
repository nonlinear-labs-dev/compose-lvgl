#pragma once
#include <string>
#include <nltools/StringTools.h>

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
      return nltools::string::concat("padding: ", std::to_string(top), "px ", std::to_string(right), "px ",
                                    std::to_string(bottom), "px ", std::to_string(left), "px;");
    }

    int left;
    int right;
    int top;
    int bottom;
  };
}
