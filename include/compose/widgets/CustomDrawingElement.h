#pragma once
#include "reactive/Var.h"
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
    using Widget::setModifier;
    using Widget::Widget;

    using tDrawCB = std::function<void(DrawContext&, int, int)>;

    explicit CustomDrawingElement(WidgetType* w)
        : Widget(w)
    {
    }

    explicit CustomDrawingElement(Widget& parent)
        : Widget(lv_canvas_create(parent.getHandle()))
    {
      applyDefaultStyle(BaseWidget::getHandle());
    }

    template <typename... tArgs>
    explicit CustomDrawingElement(Widget& parent, tArgs&&... args)
        : CustomDrawingElement(parent)
    {
      setModifiers(this, parent, std::forward<tArgs>(args)...);
    }

    void setDrawCall(tDrawCB&& draw) const;

    struct
    {
      void operator<<(tDrawCB&& cb) const
      {
        m_parent->setDrawCall(std::move(cb));
      }

      CustomDrawingElement* m_parent;
    } render { this };
  };
}

#define CANVAS(...) it.add(Compose::CustomDrawingElement(it __VA_OPT__(, __VA_ARGS__))) << [=](Compose::CustomDrawingElement&& it)
#define RENDER it.render << [=]
