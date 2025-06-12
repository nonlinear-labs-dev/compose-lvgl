#pragma once
#include <nltools/StringTools.h>

namespace Compose
{
  struct RoundedCorner
  {
    static constexpr auto key = "_RoundedCorner";

    explicit RoundedCorner(int r)
        : topLeft { r }
        , topRight { r }
        , bottomRight { r }
        , bottomLeft { r }
    {
    }

    RoundedCorner(int l, int r, int t, int b)
        : topLeft { l }
        , topRight { r }
        , bottomRight { t }
        , bottomLeft { b }
    {
    }
    int topLeft;
    int topRight;
    int bottomRight;
    int bottomLeft;

    [[nodiscard]] std::string cssRuleText() const
    {
      return nltools::string::concat(
          "border-radius: ", std::to_string(topLeft) + "px ", std::to_string(topRight) + "px ",
          std::to_string(bottomRight) + "px ", std::to_string(bottomLeft) + "px;");
    }
  };
}
