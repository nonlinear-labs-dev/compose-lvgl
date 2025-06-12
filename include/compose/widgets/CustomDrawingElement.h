#pragma once
#include <gtkmm/drawingarea.h>
#include <compose/widgets/Widget.h>

namespace Compose
{
  class DrawContext;
}

namespace Compose
{
  class DrawObject;

  class CustomDrawingElement : public Widget<Gtk::DrawingArea>
  {
   public:
    using tDrawCB = std::function<void(DrawContext &, int, int)>;

    template <typename... tArgs>
    explicit CustomDrawingElement(tArgs... args)
        : Widget(Gtk::make_managed<Gtk::DrawingArea>())
    {
      (setModifier(args), ...);
    }

    explicit CustomDrawingElement(WidgetType *handle);
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

#define CANVAS(...) it.add(Compose::CustomDrawingElement(__VA_ARGS__)) << [=](Compose::CustomDrawingElement &&it)
#define RENDER it.render << [=]
