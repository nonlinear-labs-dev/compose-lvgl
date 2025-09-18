#pragma once

namespace Compose
{
  struct OverflowBehaviour
  {
    enum
    {
      VISIBLE,
      HIDDEN
    } it;

    bool operator==(const OverflowBehaviour &) const = default;
  };
}
