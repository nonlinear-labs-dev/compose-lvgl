#pragma once
#include <compose/state/SVGData.h>

#include "CustomDrawingElement.h"

namespace Compose
{
  class SVGRenderer : public CustomDrawingElement
  {
   public:
    using CustomDrawingElement::CustomDrawingElement;
    using CustomDrawingElement::setModifier;

    explicit SVGRenderer(Widget& parent)
        : CustomDrawingElement(parent)
    {
    }

    template <typename... tArgs>
    explicit SVGRenderer(Widget& parent, tArgs&&... args)
        : SVGRenderer(parent)
    {
      (setModifier(std::forward<tArgs>(args)), ...);
    }

    void setModifier(const SVGPath& path) const;
    void setModifier(const SVGFileContent& content) const;
    void setModifier(PrimaryColor col) const override;
  };
}

#define SVG_RENDERER(...) it.add(Compose::SVGRenderer(it __VA_OPT__(, __VA_ARGS__))) << [=](Compose::SVGRenderer && it)
