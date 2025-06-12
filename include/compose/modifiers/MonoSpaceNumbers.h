#pragma once

namespace Compose
{
  struct MonoSpaceNumbers
  {
    const bool it = true;

    static constexpr MonoSpaceNumbers ON()
    {
      return MonoSpaceNumbers{ true };
    }

    static constexpr MonoSpaceNumbers OFF()
    {
      return MonoSpaceNumbers { false };
    }
  };
} 