#pragma once

enum class Justification : int
{
  LEFT,
  RIGHT,
  CENTER,
  FILL
};

namespace Compose
{
  struct Justify
  {
    Justification it;
    static Justify LEFT()
    {
      return { Justification::LEFT };
    }
    static Justify RIGHT()
    {
      return { Justification::RIGHT };
    }
    static Justify CENTER()
    {
      return { Justification::CENTER };
    }
    static Justify FILL()
    {
      return { Justification::FILL };
    }
  };
}