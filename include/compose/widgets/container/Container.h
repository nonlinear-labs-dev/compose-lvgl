#pragma once
#include "compose/widgets/Widget.h"

namespace Compose
{
  class Window;
  class Container : public Widget
  {
   public:
    using Widget::add;
    using Widget::setModifier;

    template <typename... tArgs>
    explicit Container(Window &it, tArgs... args)
        : Widget(lv_obj_create(nullptr))
    {
      lv_screen_load(Widget::getHandle());
      lv_obj_set_layout(Widget::getHandle(), LV_LAYOUT_FLEX);
      (setModifier(args), ...);
    }

    template <typename... tArgs>
    explicit Container(BaseWidget &w, tArgs... args)
        : Widget(lv_obj_create(w.getHandle()))
    {
      lv_obj_set_layout(Widget::getHandle(), LV_LAYOUT_FLEX);
      (setModifier(args), ...);
    }

    explicit Container(WidgetType *handle);

    template <typename... tArgs> static Container _ROW(tArgs... args)
    {
      auto cont = Container(args..., Orientation::HORIZONTAL());
      return cont;
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

    void setModifier(FixedSize r) const;
    void setModifier(Spacing r) const;
    void setModifier(Orientation r) const;
  };
}

#define CONTAINER(...) it.add(Compose::Container(it __VA_OPT__(, __VA_ARGS__))) << [=](Compose::Container &&it)
#define ROW(...)                                                                                                       \
  it.add(std::move(Compose::Container::_ROW(it __VA_OPT__(, __VA_ARGS__)))) << [=](Compose::Container &&it)
#define COLUMN(...)                                                                                                    \
  it.add(std::move(Compose::Container::_COLUMN(it __VA_OPT__(, __VA_ARGS__)))) << [=](Compose::Container &&it)
#define VSPACER(...) it.add(std::move(Compose::Container::V_SPACER(it __VA_OPT__(, __VA_ARGS__))));
#define HSPACER(...) it.add(std::move(Compose::Container::H_SPACER(it __VA_OPT__(, __VA_ARGS__))));
