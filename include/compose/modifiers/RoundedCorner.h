#pragma once
#include <format>

namespace Compose
{
  struct RoundedCorner
  {
    static constexpr auto key = "_RoundedCorner";

    explicit RoundedCorner(int r)
        : radius(r)
    {
    }

    int radius;
  };
}
