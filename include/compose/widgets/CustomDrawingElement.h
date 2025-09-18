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
    explicit CustomDrawingElement(Widget& parent, tArgs... args)
        : CustomDrawingElement(parent)
    {
      (setModifier(args), ...);
    }

    void setDrawCall(tDrawCB&& draw) const;
    void cleanup() const;

    void clear() override
    {
      cleanup();
      Widget::clear();
    }
    struct
    {
      void operator<<(tDrawCB&& cb) const
      {
        m_parent->setDrawCall(std::move(cb));
      }

      CustomDrawingElement* m_parent;
    } render { this };
  };

  struct CanvasData
  {
    Reactive::Var<std::unique_ptr<lv_draw_buf_t, decltype(&lv_draw_buf_destroy)>> buffer { { nullptr,
                                                                                             lv_draw_buf_destroy } };
    Reactive::Var<BackgroundColor> bgColor;
    Reactive::Var<Font> font;
    Reactive::Var<Text> text;
    Reactive::Var<TextAlign> align;
    Reactive::Var<PrimaryColor> primaryColor;

    CustomDrawingElement::tDrawCB drawCallback;
    lv_event_dsc_t* resizeHandler = nullptr;
    lv_obj_t* handle = nullptr;

    CanvasData(lv_obj_t* handle, CustomDrawingElement::tDrawCB cb);
    ~CanvasData();
    void ensureBuffer();
  };
}

#define CANVAS(...)                                                                                                    \
  it.add(Compose::CustomDrawingElement(it __VA_OPT__(, __VA_ARGS__))) << [=](Compose::CustomDrawingElement&& it)
#define RENDER it.render << [=]
