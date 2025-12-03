#pragma once
#include "Size.h"

namespace Compose
{
  struct Point
  {
    int x = {};
    int y = {};
  };

  struct Rect
  {
    Point pos;
    Size size {};
    [[nodiscard]] Rect biggerBy(int i) const;
  };

  inline Rect Rect::biggerBy(int i) const
  {
    Rect ret = *this;
    ret.size.w += (i * 2);
    ret.size.h += (i * 2);
    ret.pos.x -= i;
    ret.pos.y -= i;
    return ret;
  }
}
