#pragma once

#include <vector>

namespace Compose
{
  struct Border
  {
    int width;
    Color color;

    bool operator==(const Border &) const = default;
  };

  struct BorderSides
  {
    enum Side
    {
      TOP,
      BOTTOM,
      LEFT,
      RIGHT
    };

    std::vector<Side> sides;

    bool operator==(const BorderSides &) const = default;
  };
}
