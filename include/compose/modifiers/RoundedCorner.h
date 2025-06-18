#pragma once
#include <format>

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
      return std::format("border-radius: {}px {}px {}px {}px;", topLeft, topRight, bottomRight, bottomLeft);
    }
  };
}
