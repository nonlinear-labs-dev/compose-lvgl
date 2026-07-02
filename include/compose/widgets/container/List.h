#pragma once

#include "Container.h"
#include "compose/widgets/Widget.h"

#include <functional>
#include <string>
#include <utility>

namespace Compose
{
  class VisibleListItems;
  using ItemId = std::string;
  using ItemBuilder = std::function<void(Widget &, const ItemId &)>;
  using EmptyListPlaceholderBuilder = std::function<void(Widget &)>;

  enum class Axis
  {
    Vertical,
    Horizontal
  };

  class ListItemContainer : public Widget
  {
   public:
    using Widget::setModifier;
    using Widget::WidgetType;

    template <typename... tArgs>
    explicit ListItemContainer(Widget &parent, Axis axis, tArgs... args)
        : Widget(parent, Padding { 0 }, Scrollable::NO_SCROLL(), SizeVariant::FIT_CONTENT(), std::forward<tArgs>(args)...)
    {
      makeState(axis);
    }

    explicit ListItemContainer(WidgetType *w);

    struct State
    {
      explicit State(const ListItemContainer &container, Axis axis);
      void setChildId(const ItemBuilder &itemBuilder, const std::optional<ItemId> &id);
      lv_obj_t *handle = nullptr;
      std::optional<ItemId> childId;
      Axis axis;
    };

    void makeState(Axis axis);
    State &ensureState() const;
    void setChildId(const ItemBuilder &itemBuilder, const std::optional<ItemId> &id);
    std::optional<ItemId> getChildId() const;
  };

  class VisibleListItems : public Flex
  {
   public:
    using Flex::Flex;
    using Widget::setModifier;
    using Widget::WidgetType;

    template <typename... tArgs>
    explicit VisibleListItems(Widget &parent, Axis axis, tArgs... args)
        : Flex(parent, Scrollable::NO_SCROLL(), axis == Axis::Horizontal ? FlexFlow::HORIZONTAL() : FlexFlow::VERTICAL(),
               axis == Axis::Horizontal ? Expand::VERTICAL() : Expand::HORIZONTAL(), SizeVariant::FIT_CONTENT(), args...)
    {
      makeState(axis);
    }

    void bruteForceUpdate(int32_t parentExtend, int32_t scrollOffset, const std::vector<ItemId> &idMap, const ItemBuilder &itemBuilder,
                          const EmptyListPlaceholderBuilder &emptyListPlaceholderBuilder);
    int32_t getItemExtend() const;

    struct State
    {
      State(const VisibleListItems &items, Axis axis);
      void setupChildren(int numItemsVisible) const;
      void bruteForceUpdate(int32_t parentExtend, int scrollOffset, const std::vector<ItemId> &idMap, const ItemBuilder &itemBuilder,
                            const EmptyListPlaceholderBuilder &emptyListPlaceholderBuilder) const;
      lv_obj_t *findItemFor(const std::optional<ItemId> &itemId) const;
      lv_obj_t *findItemForRecycling(const auto &range) const;
      int32_t getItemExtend() const;
      lv_obj_t *handle = nullptr;
      Axis axis;
    };

    State &ensureState() const;
    void makeState(Axis axis);
  };

  class ListPane : public Widget
  {
   public:
    using Widget::setModifier;
    using Widget::WidgetType;

    template <typename... tArgs>
    explicit ListPane(Widget &parent, Axis axis, tArgs... args)
        : Widget(parent, Padding { 0 }, LayoutType::none(), args...)
    {
      makeState(axis);
    }

    explicit ListPane(WidgetType *w);

    bool shouldClearBeforeAutorunCompose() const override;
    void bruteForceUpdate(int32_t parentExtend, int32_t scrollOffset, const std::vector<ItemId> &idMap, const ItemBuilder &itemBuilder,
                          const EmptyListPlaceholderBuilder &emptyListPlaceholderBuilder);

    struct State
    {
      State(ListPane &pane, Axis axis);
      void bruteForceUpdate(int32_t parentExtend, int32_t scrollOffset, const std::vector<ItemId> &idMap, const ItemBuilder &itemBuilder,
                            const EmptyListPlaceholderBuilder &emptyListPlaceholderBuilder);

      lv_obj_t *handle = nullptr;
      VisibleListItems visibleItems;
      Axis axis;
    };

    void makeState(Axis axis);
    State &ensureState();
  };

  class List : public Widget
  {
   public:
    using Widget::WidgetType;

    using ItemBuilderById = std::function<void(Widget &, const ItemId &)>;
    using EmptyListPlaceholderBuilder = std::function<void(Widget &)>;
    using Widget::setModifier;

    template <typename... tArgs>
    explicit List(Widget &parent, Axis axis, tArgs... args)
        : Widget(parent, Scrollable::SCROLL(), std::forward<tArgs>(args)...)
    {
      makeState(axis);

      auto &state = ensureState();
      lv_obj_add_event_cb(getHandle(), onListChanged, LV_EVENT_SCROLL, &state);
      lv_obj_add_event_cb(getHandle(), onListChanged, LV_EVENT_SIZE_CHANGED, &state);
    }

    explicit List(WidgetType *w);

    struct State
    {
      State(Widget &list, Axis axis);

      lv_obj_t *handle = nullptr;
      std::vector<ItemId> idMap;
      ListPane pane;
      Axis axis;

      ItemBuilder itemBuilder;
      EmptyListPlaceholderBuilder emptyListPlaceholderBuilder;

      void setIdMap(std::vector<ItemId> idMap);
      void scrollTo(const ItemId &id);

      void setItemBuilder(auto &&cb)
      {
        itemBuilder = std::move(cb);
        bruteForceUpdate();
      }

      void setEmptyListPlaceholderBuilder(auto &&cb)
      {
        emptyListPlaceholderBuilder = std::move(cb);
        bruteForceUpdate();
      }

      void bruteForceUpdate();
    };

    void makeState(Axis axis);
    State &ensureState();
    bool shouldClearBeforeAutorunCompose() const override;
    static void onListChanged(lv_event_t *e);

    struct ItemIds
    {
      List *m_parent;

      template <typename CB> void operator<<(CB &&cb) const
      {
        m_parent->doAutorun([h = m_parent->getHandle(), cb = std::move(cb)] { List(h).ensureState().setIdMap(cb()); });
      }
    } itemIds { this };

    struct ItemById
    {
      List *m_parent;

      template <typename CB> void operator<<(CB &&cb) const
      {
        m_parent->ensureState().setItemBuilder(std::move(cb));
      }
    } itemById { this };

    struct PlaceHolder
    {
      List *m_parent;

      template <typename CB> void operator<<(CB &&cb) const
      {
        m_parent->ensureState().setEmptyListPlaceholderBuilder(std::move(cb));
      }
    } emptyListPlaceholder { this };

    struct ScrollTo
    {
      List *m_parent;

      template <typename CB> void operator<<(CB &&cb) const
      {
        m_parent->doAutorun([h = m_parent->getHandle(), cb = std::move(cb)] { List(h).ensureState().scrollTo(cb()); });
      }
    } scrollTo { this };
  };
}

#define LIST(...) it.add(std::move(Compose::List(it __VA_OPT__(, __VA_ARGS__)))) << [=](Compose::List &&it)
#define LIST_ITEM_IDS() it.itemIds << [=]()
#define LIST_ITEM_BY_ID it.itemById << [=]
#define SCROLL_TO_ITEM() it.scrollTo << [=]()
#define EMPTY_LIST_PLACEHOLDER() it.emptyListPlaceholder << [=](Compose::Widget & it)