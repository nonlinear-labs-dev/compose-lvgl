#pragma once
#include <compose/widgets/Widget.h>
#include "src/widgets/image/lv_image.h"

namespace Compose
{
  class SVGImage : public Widget
  {
   public:
    using Widget::setModifier;
    using Widget::Widget;

    template <typename... tArgs>
    explicit SVGImage(BaseWidget& parent, tArgs... args)
        : Widget(lv_image_create(parent.getHandle()))
    {
      (setModifier(args), ...);
    }

    void setModifier(const SVGPath& p) const;
    void setModifier(PixelSize s) const;
    void setModifier(PrimaryColor col) const;
  };
}

#define IMAGE(...) it.add(Compose::SVGImage(it __VA_OPT__(, __VA_ARGS__))) << [=](Compose::SVGImage && it)