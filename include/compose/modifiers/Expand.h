#pragma once

namespace Compose
{
  struct Expand
  {
    bool vertical = false;
    bool horizontal = false;

    static Expand VERTICAL()
    {
      return Expand(true, false);
    }

    static Expand HORIZONTAL()
    {
      return Expand(false, true);
    }

    static Expand NONE()
    {
      return Expand(false, false);
    }

    static Expand BOTH()
    {
      return Expand(true, true);
    }

    bool operator==(const Expand &) const = default;
  };
}