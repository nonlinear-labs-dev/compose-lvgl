#pragma once

#include "Color.h"

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

  struct BorderWidth
  {
    BorderWidth() = default;

    constexpr explicit BorderWidth(int w = 0)
        : width { w }
    {
    }

    int width;
    bool operator==(const BorderWidth &) const = default;
  };

  struct BorderColor
  {
    BorderColor() = default;

    constexpr explicit BorderColor(const Color &c)
        : color { c }
    {
    }

    Color color;
    bool operator==(const BorderColor &) const = default;
  };
}
