#pragma once
#include "compose/widgets/Widget.h"
#include <gtkmm.h>

namespace Compose
{
  class Container : public Widget<Gtk::Box>
  {
   public:
    using Widget::setModifier;

    template <typename... tArgs> static Container _ROW(tArgs... args)
    {
      return Container(args..., Orientation::HORIZONTAL());
    }

    template <typename... tArgs> static Container _COLUMN(tArgs... args)
    {
      return Container(args..., Orientation::VERTICAL());
    }

    template <typename... tArgs> static Container V_SPACER(tArgs... args)
    {
      return _ROW(args..., Expand::VERTICAL());
    }

    template <typename... tArgs> static Container H_SPACER(tArgs... args)
    {
      return _COLUMN(args..., Expand::HORIZONTAL());
    }

    template <typename... tArgs>
    explicit Container(tArgs... args)
        : Widget(Gtk::make_managed<Gtk::Box>())
    {
      (setModifier(args), ...);
    }

    explicit Container(WidgetType *handle);
    void setModifier(FixedSize r) const;
    void setModifier(Spacing r) const;
    void clear() override;
  };
}

#define CONTAINER(...) it.add(Compose::Container(__VA_ARGS__)) << [=](Compose::Container &&it)
#define ROW(...) it.add(std::move(Compose::Container::_ROW(__VA_ARGS__))) << [=](Compose::Container &&it)
#define COLUMN(...) it.add(std::move(Compose::Container::_COLUMN(__VA_ARGS__))) << [=](Compose::Container &&it)
#define VSPACER(...) it.add(std::move(Compose::Container::V_SPACER(__VA_ARGS__)));
#define HSPACER(...) it.add(std::move(Compose::Container::H_SPACER(__VA_ARGS__)));
