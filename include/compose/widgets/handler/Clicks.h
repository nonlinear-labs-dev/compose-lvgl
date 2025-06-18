#pragma once
#include <nltools/logging/Log.h>
#include <reactive/Deferrer.h>

namespace Gtk
{
  class Widget;
}

namespace Compose
{
  template <typename tHandle> struct LeftClick
  {
    tHandle& widget;

    template <typename tCB> void operator<<(const tCB& cb)
    {
      auto handle = widget.getHandle();
      auto connection = handle->signal_clicked().connect(
          [cb = std::move(cb)]()
          {
            Reactive::Deferrer ref;
            cb();
          });

      widget.addConnection(connection);
    }
  };

  template <typename tHandle> struct RightClick
  {
    tHandle& widget;

    template <typename tCB> void operator<<(const tCB& cb)
    {
      // auto handle = widget.getHandle();
      // handle->add_events(Gdk::BUTTON_PRESS_MASK);
      // auto connection = handle->signal_button_press_event().connect(
      //     [cb](const GdkEventButton* event)
      //     {
      //       if(event->button == GDK_BUTTON_SECONDARY)
      //       {
      //         Reactive::Deferrer ref;
      //         cb();
      //         return true;
      //       }
      //       return false;
      //     });
      // widget.addConnection(connection);
    }
  };
}

#define LEFT_CLICK() it.leftClickHandler << [=]
#define RIGHT_CLICK() it.rightClickHandler << [=]