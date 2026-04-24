#pragma once
#include "Size.h"

namespace Compose
{
  struct Point
  {
    int x = {};
    int y = {};
    [[nodiscard]] Point moveByX(int i) const;
    [[nodiscard]] Point moveByY(int i) const;
    [[nodiscard]] Point moveByXY(int ix, int iy) const;
  };

  inline Point Point::moveByX(int i) const
  {
    return { x + i, y };
  }

  inline Point Point::moveByY(int i) const
  {
    return { x, y + i };
  }

  inline Point Point::moveByXY(int ix, int iy) const
  {
    return { x + ix, y + iy };
  }

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
    [[nodiscard]] Point nw() const;
    [[nodiscard]] Point ne() const;
    [[nodiscard]] Point sw() const;
    [[nodiscard]] Point se() const;
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

  inline Point Rect::nw() const
  {
    return pos;
  }

  inline Point Rect::ne() const
  {
    return pos.moveByX(size.w);
  }

  inline Point Rect::sw() const
  {
    return pos.moveByY(size.h);
  }

  inline Point Rect::se() const
  {
    return pos.moveByXY(size.w, size.h);
  }
}
