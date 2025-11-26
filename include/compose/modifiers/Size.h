#pragma once

namespace Compose
{
  struct Size
  {

    template <typename T> constexpr Size operator*(T v) const
    {
      return Size { w * v, h * v };
    }

    constexpr Size operator+(const Size &size) const
    {
      return Size { w + size.w, h + size.h };
    }

    int w = {};
    int h = {};

    bool operator==(const Size &) const = default;
  };

  struct FixedSize : Size
  {
    bool operator==(const FixedSize &) const = default;
  };
}