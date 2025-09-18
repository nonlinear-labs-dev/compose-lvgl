#pragma once

enum class OrientationEnum : int
{
  HORIZONTAL,
  VERTICAL
};

namespace Compose
{
  struct Orientation
  {
    OrientationEnum it;

    static Orientation HORIZONTAL()
    {
      return { OrientationEnum::HORIZONTAL };
    }
    static Orientation VERTICAL()
    {
      return { OrientationEnum::VERTICAL };
    }

    bool operator==(const Orientation &) const = default;
  };
}