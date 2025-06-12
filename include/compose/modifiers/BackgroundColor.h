#pragma once
#include "Color.h"

namespace Compose
{
  struct BackgroundColor : Color
  {
    static constexpr auto key = "_BackgroundColor";
  };
}