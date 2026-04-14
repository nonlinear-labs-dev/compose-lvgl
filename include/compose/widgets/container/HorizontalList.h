#pragma once

#include "compose/widgets/container/VirtualizedList.h"

namespace Compose
{
  class HorizontalList : public VirtualizedList
  {
   public:
    using VirtualizedList::setModifier;

    template <typename... tArgs>
    explicit HorizontalList(Widget &parent, tArgs... args)
        : VirtualizedList(parent, Axis::Horizontal, args...)
    {
    }

    explicit HorizontalList(WidgetType *w)
        : VirtualizedList(w, Axis::Horizontal)
    {
    }

    void setModifier(ItemWidth width) const
    {
      setItemExtent(width.it);
    }
  };
}

#define HORIZONTAL_LIST(...) it.add(Compose::HorizontalList(it __VA_OPT__(, __VA_ARGS__))) << [=](Compose::HorizontalList && it)
