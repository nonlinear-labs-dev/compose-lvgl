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

    void scrollToItem(size_t index) const override
    {
      const auto itemHeight = getItemExtent();
      const int itemTop = itemHeight * index;
      const int itemBottom = itemTop + itemHeight;
      const int scrollTop = lv_obj_get_scroll_top(getHandle());
      const int scrollBottom = scrollTop + lv_obj_get_height(getHandle());

      if(itemTop < scrollTop)
        lv_obj_scroll_by(getHandle(), 0, scrollTop - itemTop, LV_ANIM_OFF);
      else if(itemBottom > scrollBottom)
        lv_obj_scroll_by(getHandle(), 0, scrollBottom - itemBottom, LV_ANIM_OFF);
    }
  };
}

#define VERTICAL_LIST(...)                                                                                             \
  it.add(Compose::VerticalList(it __VA_OPT__(, __VA_ARGS__))) << [=](Compose::VerticalList &&it)
