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
    [[nodiscard]] Point midPoint(const Point p) const;
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

  inline Point Point::midPoint(const Point p) const
  {
    const int dx = p.x - x, dy = p.y - y;
    return moveByXY(dx >> 1, dy >> 1);
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
    [[nodiscard]] Point northWest() const;
    [[nodiscard]] Point north() const;
    [[nodiscard]] Point northEast() const;
    [[nodiscard]] Point east() const;
    [[nodiscard]] Point southEast() const;
    [[nodiscard]] Point south() const;
    [[nodiscard]] Point southWest() const;
    [[nodiscard]] Point west() const;
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

  inline Point Rect::northWest() const
  {
    return pos;
  }

  inline Point Rect::north() const
  {
    return pos.moveByX(size.w >> 1);
  }

  inline Point Rect::northEast() const
  {
    return pos.moveByX(size.w);
  }

  inline Point Rect::east() const
  {
    return pos.moveByXY(size.w, size.h >> 1);
  }

  inline Point Rect::southEast() const
  {
    return pos.moveByXY(size.w, size.h);
  }

  inline Point Rect::south() const
  {
    return pos.moveByXY(size.w >> 1, size.h);
  }

  inline Point Rect::southWest() const
  {
    return pos.moveByY(size.h);
  }

  inline Point Rect::west() const
  {
    return pos.moveByY(size.h >> 1);
  }
}
