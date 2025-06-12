#pragma once
#include <pangomm/layout.h>
#include <nltools/enums/EnumDecl.h>

ENUM(EllipsizeMode, int, NONE = Pango::EllipsizeMode::ELLIPSIZE_NONE, START = Pango::EllipsizeMode::ELLIPSIZE_START,
     MIDDLE = Pango::EllipsizeMode::ELLIPSIZE_MIDDLE, END = Pango::EllipsizeMode::ELLIPSIZE_END);

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