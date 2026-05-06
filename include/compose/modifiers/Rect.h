#pragma once
#include "Size.h"

namespace Compose
{
  struct Point
  {
    int x = {};
    int y = {};
    [[nodiscard]] Point shiftedHorizontallyBy(int i) const;
    [[nodiscard]] Point shiftedVerticallyBy(int i) const;
    [[nodiscard]] Point shiftedBy(int ix, int iy) const;
    [[nodiscard]] Point midPointWith(const Point p) const;
  };

  inline Point Point::shiftedHorizontallyBy(int i) const
  {
    return { x + i, y };
  }

  inline Point Point::shiftedVerticallyBy(int i) const
  {
    return { x, y + i };
  }

  inline Point Point::shiftedBy(int ix, int iy) const
  {
    return { x + ix, y + iy };
  }

  inline Point Point::midPointWith(const Point p) const
  {
    const int dx = p.x - x, dy = p.y - y;
    return shiftedBy(dx >> 1, dy >> 1);
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

    // anchor points
    [[nodiscard]] Point topLeft() const;
    [[nodiscard]] Point top() const;
    [[nodiscard]] Point topRight() const;
    [[nodiscard]] Point right() const;
    [[nodiscard]] Point bottomRight() const;
    [[nodiscard]] Point bottom() const;
    [[nodiscard]] Point bottomLeft() const;
    [[nodiscard]] Point left() const;
    [[nodiscard]] Point center() const;
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

  inline Point Rect::topLeft() const
  {
    return pos;
  }

  inline Point Rect::top() const
  {
    return pos.shiftedHorizontallyBy(size.w >> 1);
  }

  inline Point Rect::topRight() const
  {
    return pos.shiftedHorizontallyBy(size.w);
  }

  inline Point Rect::right() const
  {
    return pos.shiftedBy(size.w, size.h >> 1);
  }

  inline Point Rect::bottomRight() const
  {
    return pos.shiftedBy(size.w, size.h);
  }

  inline Point Rect::bottom() const
  {
    return pos.shiftedBy(size.w >> 1, size.h);
  }

  inline Point Rect::bottomLeft() const
  {
    return pos.shiftedVerticallyBy(size.h);
  }

  inline Point Rect::left() const
  {
    return pos.shiftedVerticallyBy(size.h >> 1);
  }

  inline Point Rect::center() const
  {
    return pos.shiftedBy(size.w >> 1, size.h >> 1);
  }
}
