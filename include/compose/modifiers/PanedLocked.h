#pragma once

namespace Compose
{
  struct PanedLocked
  {
    bool it;
    
    constexpr static PanedLocked ON()
    {
      return { true };
    }
    
    constexpr static PanedLocked OFF()
    {
      return { false };
    }
  };
} 