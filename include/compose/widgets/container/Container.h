#pragma once
#include "compose/widgets/Widget.h"

namespace Compose
{
  namespace Container
  {
    template <typename... tArgs> static Widget _ROW(tArgs... args)
    {
      return Widget(args..., Orientation::HORIZONTAL());
    }

    template <typename... tArgs> static Widget _COLUMN(tArgs... args)
    {
      return Widget(args..., Orientation::VERTICAL());
    }

    template <typename... tArgs> static Widget V_SPACER(tArgs... args)
    {
      return _ROW(args..., Expand::VERTICAL());
    }

    template <typename... tArgs> static Widget H_SPACER(tArgs... args)
    {
      return _COLUMN(args..., Expand::HORIZONTAL());
    }
  };
}

#define CONTAINER(...) it.add(Compose::Widget(it __VA_OPT__(, __VA_ARGS__))) << [=](Compose::Widget &&it)
#define ROW(...)                                                                                                       \
  it.add(std::move(Compose::Container::_ROW(it __VA_OPT__(, __VA_ARGS__)))) << [=](Compose::Widget &&it)
#define COLUMN(...)                                                                                                    \
  it.add(std::move(Compose::Container::_COLUMN(it __VA_OPT__(, __VA_ARGS__)))) << [=](Compose::Widget &&it)
// #define VSPACER(...) it.add(std::move(Compose::Container::V_SPACER(it __VA_OPT__(, __VA_ARGS__))));
// #define HSPACER(...) it.add(std::move(Compose::Container::H_SPACER(it __VA_OPT__(, __VA_ARGS__))));

#define VSPACER(...) ;
#define HSPACER(...) ;