#pragma once
#include "Color.h"

namespace Compose
{
  struct BackgroundColor : Color
  {
    static constexpr auto key = "_BackgroundColor";

    static constexpr BackgroundColor TRANSPARENT()
    {
      return BackgroundColor { Color::TRANSPARENT() };
    }
  };
}