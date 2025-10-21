#pragma once
#include "Color.h"

namespace Compose
{
  struct PrimaryColor : Color
  {
    static constexpr auto key = "_PrimaryColor";
    bool operator==(const PrimaryColor &) const = default;

    constexpr static PrimaryColor fromHEXString(const std::string &hexString)
    {
      return { Color::fromHEXString(hexString) };
    }
  };
}