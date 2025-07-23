#pragma once
#include "compose/widgets/Widget.h"

namespace Compose
{
  class Flex : public Widget
  {
   public:
    using Widget::add;
    using Widget::setModifier;
    using Widget::Widget;
    using Widget::WidgetType;
  };

  class Row : public Flex
  {
   public:
    using Flex::add;
    using Flex::Flex;
    using Flex::setModifier;
    using Flex::WidgetType;
  };

  class Column : public Flex
  {
   public:
    using Flex::add;
    using Flex::Flex;
    using Flex::setModifier;
    using Flex::WidgetType;
  };

  template <typename... tArgs> static Row _ROW(tArgs... args)
  {
    return Row(args..., Orientation::HORIZONTAL());
  }

  template <typename... tArgs> static Column _COLUMN(tArgs... args)
  {
    return Column(args..., Orientation::VERTICAL());
  }

  class Fixed : public Widget
  {
   public:
    using Widget::setModifier;
    using Widget::Widget;
  };

  template <typename... tArgs> static Fixed _FIXED(tArgs... args)
  {
    return Fixed(args..., Orientation::VERTICAL(), LayoutType::none());
  }

  template <typename... tArgs> static Widget V_SPACER(tArgs... args)
  {
    return _ROW(args..., Expand::VERTICAL());
  }

  template <typename... tArgs> static Widget H_SPACER(tArgs... args)
  {
    return _COLUMN(args..., Expand::HORIZONTAL());
  }
}

#define ROW(...) it.add(std::move(Compose::_ROW(it __VA_OPT__(, __VA_ARGS__)))) << [=](Compose::Row &&it)
#define COLUMN(...) it.add(std::move(Compose::_COLUMN(it __VA_OPT__(, __VA_ARGS__)))) << [=](Compose::Column &&it)
#define FIXED_CONTAINER(...)                                                                                           \
  it.add(std::move(Compose::_FIXED(it __VA_OPT__(, __VA_ARGS__)))) << [=](Compose::Fixed &&it)

#define VSPACER(...) ;
#define HSPACER(...) ;