#pragma once
#include "src/widgets/canvas/lv_canvas.h"
#include <compose/widgets/Widget.h>

namespace Compose
{
  class DrawContext;
}

namespace Compose
{
  class DrawObject;

  class CustomDrawingElement : public Widget
  {
   public:
    using Widget::Widget;

    using tDrawCB = std::function<void(DrawContext &, int, int)>;
    template <typename... tArgs>
    explicit CustomDrawingElement(Widget &parent, tArgs... args)
        : Widget(lv_canvas_create(parent.getHandle()))
    {
      lv_obj_set_style_margin_all(getHandle(), 0, 0);
      (setModifier(args), ...);
    }

    void setDrawCall(tDrawCB &&draw) const;
    struct
    {
      void operator<<(tDrawCB &&cb) const
      {
        m_parent->setDrawCall(std::move(cb));
      }

      CustomDrawingElement *m_parent;
    } render { this };
  };
}

#define CANVAS(...)                                                                                                    \
  it.add(Compose::CustomDrawingElement(it __VA_OPT__(, __VA_ARGS__))) << [=](Compose::CustomDrawingElement && it)
#define RENDER it.render << [=]
