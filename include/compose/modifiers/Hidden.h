#pragma once

namespace Compose
{
  struct Hidden
  {
    constexpr static auto key = "_displayNone";
    const bool it = true;

    static Hidden ON()
    {
      return Hidden(true);
    }

    static Hidden OFF()
    {
      return Hidden(false);
    }
  };
} 