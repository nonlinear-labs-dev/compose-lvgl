#pragma once

#include "compose/widgets/Widget.h"

#include <functional>
#include <utility>

namespace Compose
{
  struct Overscan
  {
    int numItems = 2;
  };

  class VirtualizedList : public Widget
  {
   public:
    using ItemBuilder = std::function<void(Widget &, int)>;
    using Widget::setModifier;

    enum class Axis
    {
      Vertical,
      Horizontal
    };

    template <typename... tArgs>
    explicit VirtualizedList(Widget &parent, Axis axis, tArgs... args)
        : Widget(parent, axis == Axis::Vertical ? FlexFlow::VERTICAL() : FlexFlow::HORIZONTAL(), Scrollable::SCROLL())
        , m_axis(axis)
    {
      ensureState();
      (setModifier(args), ...);
    }

    explicit VirtualizedList(WidgetType *w, Axis axis);

    void clear() override;
    void setModifier(ItemCount count) const;
    void setModifier(Overscan overscanItems) const;
    void setModifier(FlexAlign align) const;
    void setModifier(const Style &style) const;
    void setItemBuilder(ItemBuilder cb) const;
    virtual void scrollToItem(size_t index) const = 0;

    template <typename CB> void setItemBuilder(CB &&cb) const
    {
      setItemBuilder(ItemBuilder { std::forward<CB>(cb) });
    }

    struct ItemDefinition
    {
      template <typename CB> void operator<<(CB &&cb) const
      {
        m_parent->setItemBuilder(std::forward<CB>(cb));
      }

      const VirtualizedList *m_parent;
    } item { this };

   protected:
    void setItemExtent(int extent) const;
    int getItemExtent() const;

   private:
    struct State;

    static constexpr auto c_virtualListStateKey = "VirtualListState";
    const Axis m_axis;

    State &ensureState() const;
  };
}

#define LIST_ITEM(...) it.item << [=](Compose::Widget & it, __VA_ARGS__)
