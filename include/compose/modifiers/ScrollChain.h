#pragma once

namespace Compose
{
  struct ScrollChain
  {
    enum
    {
      CHAINED,
      ISOLATED
    } it;

    bool operator==(const ScrollChain &) const = default;
  };
}
