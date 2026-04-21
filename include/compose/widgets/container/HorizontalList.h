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

    void scrollToItem(size_t index) const override
    {
      auto itemWidth = getItemExtent();
      auto itemLeft = itemWidth * index;
      auto itemRight = itemLeft + itemWidth;
      auto scrollLeft = lv_obj_get_scroll_left(getHandle());
      auto scrollRight = scrollLeft + lv_obj_get_width(getHandle());

      if(itemLeft < scrollLeft)
        lv_obj_scroll_by(getHandle(), scrollLeft - itemLeft, 0, LV_ANIM_ON);
      else if(itemRight > scrollRight)
        lv_obj_scroll_by(getHandle(), scrollRight - itemRight, 0, LV_ANIM_ON);
    }
  };
}

#define HORIZONTAL_LIST(...) it.add(Compose::HorizontalList(it __VA_OPT__(, __VA_ARGS__))) << [=](Compose::HorizontalList && it)
