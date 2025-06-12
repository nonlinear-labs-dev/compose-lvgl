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
  };
}
