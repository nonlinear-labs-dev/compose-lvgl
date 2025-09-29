#pragma once
#include "CustomDrawingElement.h"
#include "compose/state/SVGData.h"
#include <string>

namespace Compose
{
  class SVGRenderer : public CustomDrawingElement
  {
   public:
    using CustomDrawingElement::setModifier;
    using CustomDrawingElement::CustomDrawingElement;

    explicit SVGRenderer(Widget& parent)
        : CustomDrawingElement(parent)
    {
    }

    template <typename... tArgs>
    explicit SVGRenderer(Widget& parent, tArgs... args)
        : SVGRenderer(parent)
    {
      (setModifier(args), ...);
    }

    void setSVG(const std::string& svgContent);
    void setSVGFile(const std::string& filePath);
  };
}

#define SVG_RENDERER(...)                                                                                              \
  it.add(Compose::SVGRenderer(it __VA_OPT__(, __VA_ARGS__))) << [=](Compose::SVGRenderer&& it)
