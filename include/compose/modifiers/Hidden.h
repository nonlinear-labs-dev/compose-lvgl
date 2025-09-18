#pragma once

namespace Compose
{
  struct Hidden
  {
    bool it = true;

    static Hidden ON()
    {
      return Hidden(true);
    }

    static Hidden OFF()
    {
      return Hidden(false);
    }

    bool operator==(const Hidden &) const = default;
  };
}