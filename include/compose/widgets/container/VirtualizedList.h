#pragma once

#include "compose/widgets/Widget.h"

#include <functional>
#include <string>
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
    using ItemIdProvider = std::function<std::string(int)>;
    using ItemBuilderById = std::function<void(Widget &, const std::string &)>;
    using EmptyListPlaceholderBuilder = std::function<void(Widget &)>;
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
    void setItemIdProvider(ItemIdProvider cb) const;
    void setItemBuilderById(ItemBuilderById cb) const;
    void setEmptyListPlaceholderBuilder(EmptyListPlaceholderBuilder cb) const;
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

    struct ItemIdDefinition
    {
      template <typename CB> void operator<<(CB &&cb) const
      {
        m_parent->setItemIdProvider(std::forward<CB>(cb));
      }

      const VirtualizedList *m_parent;
    } itemId { this };

    struct ItemByIdDefinition
    {
      template <typename CB> void operator<<(CB &&cb) const
      {
        m_parent->setItemBuilderById(std::forward<CB>(cb));
      }

      const VirtualizedList *m_parent;
    } itemById { this };

    struct EmptyListPlaceholder
    {
      template <typename CB> void operator<<(CB &&cb) const
      {
        m_parent->setEmptyListPlaceholderBuilder(std::forward<CB>(cb));
      }

      const VirtualizedList *m_parent;
    } emptyListPlaceholder { this };

    [[nodiscard]] bool shouldClearBeforeAutorunCompose() const override
    {
      return false;
    }

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
#define LIST_ITEM_ID(...) it.itemId << [=](__VA_ARGS__)
#define LIST_ITEM_BY_ID(...) it.itemById << [=](Compose::Widget & it, __VA_ARGS__)
#define EMPTY_LIST_PLACEHOLDER() it.emptyListPlaceholder << [=](Compose::Widget & it)
