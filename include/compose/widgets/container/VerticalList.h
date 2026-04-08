#pragma once

#include "compose/widgets/container/VirtualizedList.h"

namespace Compose
{
  class VerticalList : public VirtualizedList
  {
   public:
    using VirtualizedList::setModifier;

    template <typename... tArgs>
    explicit VerticalList(Widget &parent, tArgs... args)
        : VirtualizedList(parent, Axis::Vertical, args...)
    {
    }

    explicit VerticalList(WidgetType *w)
        : VirtualizedList(w, Axis::Vertical)
    {
    }

    void setModifier(ItemHeight height) const
    {
      setItemExtent(height.it);
    }
  };
}

#define VERTICAL_LIST(...)                                                                                       \
  it.add(Compose::VerticalList(it __VA_OPT__(, __VA_ARGS__))) << [=](Compose::VerticalList &&it)
