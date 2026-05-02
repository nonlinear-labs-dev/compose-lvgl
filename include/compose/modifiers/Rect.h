#pragma once
#include "Size.h"

namespace Compose
{
  struct Point
  {
    int x = {};
    int y = {};
  };

  struct PointF
  {
    float x = {};
    float y = {};
  };

  struct Rect
  {
    Point pos;
    Size size {};
    [[nodiscard]] Rect biggerBy(int i) const;
    [[nodiscard]] Rect expanded(int horizontal, int vertical) const;
  };

  inline Rect Rect::biggerBy(int i) const
  {
    return expanded(i, i);
  }
  inline Rect Rect::expanded(int horizontal, int vertical) const
  {
    Rect ret = *this;
    ret.size.w += (horizontal * 2);
    ret.size.h += (vertical * 2);
    ret.pos.x -= horizontal;
    ret.pos.y -= vertical;
    return ret;
  }
}
