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

  template <typename tWidget> static Row _ROW(tWidget &it)
  {
    return Row(it, FlexFlow::HORIZONTAL());
  }

  template <typename tWidget> static Row _ROW(const tWidget &it)
  {
    return Row(const_cast<tWidget &>(it), FlexFlow::HORIZONTAL());
  }

  template <typename tWidget, typename... tArgs> static Row _ROW(tWidget &it, tArgs &&... args)
  {
    return Row(it, FlexFlow::HORIZONTAL(), std::forward<tArgs>(args)...);
  }

  template <typename tWidget, typename... tArgs> static Row _ROW(const tWidget &it, tArgs &&... args)
  {
    return Row(const_cast<tWidget &>(it), FlexFlow::HORIZONTAL(), std::forward<tArgs>(args)...);
  }

  template <typename tWidget> static Column _COLUMN(tWidget &it)
  {
    return Column(it, FlexFlow::VERTICAL());
  }

  template <typename tWidget> static Column _COLUMN(const tWidget &it)
  {
    return Column(const_cast<tWidget &>(it), FlexFlow::VERTICAL());
  }

  template <typename tWidget, typename... tArgs> static Column _COLUMN(tWidget &it, tArgs &&... args)
  {
    return Column(it, FlexFlow::VERTICAL(), std::forward<tArgs>(args)...);
  }

  template <typename tWidget, typename... tArgs> static Column _COLUMN(const tWidget &it, tArgs &&... args)
  {
    return Column(const_cast<tWidget &>(it), FlexFlow::VERTICAL(), std::forward<tArgs>(args)...);
  }

  class Fixed : public Widget
  {
   public:
    using Widget::setModifier;
    using Widget::Widget;
  };

  template <typename tWidget> static Fixed _FIXED(tWidget &it)
  {
    return Fixed(it, LayoutType::none());
  }

  template <typename tWidget> static Fixed _FIXED(const tWidget &it)
  {
    return Fixed(const_cast<tWidget &>(it), LayoutType::none());
  }

  template <typename tWidget, typename... tArgs> static Fixed _FIXED(tWidget &it, tArgs &&... args)
  {
    return Fixed(it, LayoutType::none(), std::forward<tArgs>(args)...);
  }

  template <typename tWidget, typename... tArgs> static Fixed _FIXED(const tWidget &it, tArgs &&... args)
  {
    return Fixed(const_cast<tWidget &>(it), LayoutType::none(), std::forward<tArgs>(args)...);
  }

  template <typename... tArgs> static Widget V_SPACER(tArgs &&... args)
  {
    return _ROW(std::forward<tArgs>(args)..., Expand::VERTICAL());
  }

  template <typename... tArgs> static Widget H_SPACER(tArgs &&... args)
  {
    return _COLUMN(std::forward<tArgs>(args)..., Expand::HORIZONTAL());
  }
}

#define ROW(...) it.add(std::move(Compose::_ROW(it __VA_OPT__(, __VA_ARGS__)))) << [=](Compose::Row && it)
#define COLUMN(...) it.add(std::move(Compose::_COLUMN(it __VA_OPT__(, __VA_ARGS__)))) << [=](Compose::Column && it)
#define FIXED_CONTAINER(...) it.add(std::move(Compose::_FIXED(it __VA_OPT__(, __VA_ARGS__)))) << [=](Compose::Fixed && it)

#define VSPACER(...) ;
#define HSPACER(...) ;