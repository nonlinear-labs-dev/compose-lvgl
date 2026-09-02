#pragma once

namespace Compose
{
  struct ExtClickArea
  {
    int size;

    explicit ExtClickArea(int s)
        : size { s }
    {
    }

    bool operator==(const ExtClickArea &) const = default;
  };
}
