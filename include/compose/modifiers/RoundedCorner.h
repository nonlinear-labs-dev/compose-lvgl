#pragma once

namespace Compose
{
  struct RoundedCorner
  {
    static constexpr auto key = "_RoundedCorner";

    RoundedCorner()
        : radius(0)
    {
    }

    constexpr explicit RoundedCorner(int r)
        : radius(r)
    {
    }

    int radius;

    bool operator==(const RoundedCorner &) const = default;
  };
}
