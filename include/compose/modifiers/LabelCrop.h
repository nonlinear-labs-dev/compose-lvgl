#pragma once

enum class EllipsizeMode : int
{
  NONE,
  START,
  MIDDLE,
  END
};

namespace Compose
{
  struct LabelCrop
  {
    EllipsizeMode it;
    static LabelCrop NONE()
    {
      return { EllipsizeMode::NONE };
    }
    static LabelCrop END()
    {
      return { EllipsizeMode::END };
    }
    static LabelCrop START()
    {
      return { EllipsizeMode::START };
    }
    static LabelCrop MIDDLE()
    {
      return { EllipsizeMode::MIDDLE };
    }
  };
} 