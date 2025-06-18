#pragma once
#include <nltools/enums/EnumDecl.h>

enum class Alignment : int
{
  END,
  START,
  FILL,
  CENTER,
  BASELINE
};

namespace Compose
{
  template <typename T> struct Align
  {
    Alignment it;
    constexpr static T END()
    {
      return { Alignment::END };
    }
    constexpr static T START()
    {
      return { Alignment::START };
    }
    constexpr static T CENTER()
    {
      return { Alignment::CENTER };
    }
    constexpr static T BASELINE()
    {
      return { Alignment::BASELINE };
    }
    constexpr static T FILL()
    {
      return { Alignment::FILL };
    }
  };

  struct HAlign : Align<HAlign>
  {
  };

  struct VAlign : Align<VAlign>
  {
  };
}