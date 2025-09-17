#pragma once

namespace Compose
{
  struct Scrollable
  {
    enum
    {
      SCROLLABLE,
      FIXED
    } it;

    static constexpr Scrollable NO_SCROLL()
    {
      return { FIXED };
    }

    static constexpr Scrollable SCROLL()
    {
      return { SCROLLABLE };
    }

    bool operator==(const Scrollable &) const = default;
  };
}