#pragma once

#include "Container.h"
#include "compose/widgets/Widget.h"

#include <algorithm>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace Compose
{
  using ItemId = std::string;
  template <typename TId> using BasicItemBuilder = std::function<void(Widget &, const TId &)>;
  using ItemBuilder = BasicItemBuilder<ItemId>;
  using EmptyListPlaceholderBuilder = std::function<void(Widget &)>;

  enum class Axis
  {
    Vertical,
    Horizontal
  };

  template <typename TId> struct ListModel;

  template <> struct ListModel<ItemId>
  {
    std::vector<ItemId> ids;

    size_t size() const
    {
      return ids.size();
    }

    const ItemId &at(size_t i) const
    {
      return ids[i];
    }

    std::optional<size_t> indexOf(const ItemId &id) const
    {
      auto it = std::find(ids.begin(), ids.end(), id);
      if(it != ids.end())
        return std::distance(ids.begin(), it);
      return std::nullopt;
    }
  };

  template <> struct ListModel<size_t>
  {
    size_t count = 0;

    size_t size() const
    {
      return count;
    }

    size_t at(size_t i) const
    {
      return i;
    }

    std::optional<size_t> indexOf(size_t id) const
    {
      if(id < count)
        return id;
      return std::nullopt;
    }
  };

  template <typename TId> class BasicListItemContainer : public Widget
  {
   public:
    using Widget::setModifier;
    using Widget::WidgetType;

    template <typename... tArgs>
    explicit BasicListItemContainer(Widget &parent, Axis axis, tArgs... args)
        : Widget(parent, Padding { 0 }, Scrollable::NO_SCROLL(), SizeVariant::FIT_CONTENT(), std::forward<tArgs>(args)...)
    {
      makeState(axis);
    }

    explicit BasicListItemContainer(WidgetType *w)
        : Widget(w)
    {
    }

    struct State
    {
      State(const BasicListItemContainer &container, Axis axis)
          : handle(container.getHandle())
          , axis(axis)
      {
      }

      void setChildId(const BasicItemBuilder<TId> &itemBuilder, const std::optional<TId> &id);
      lv_obj_t *handle = nullptr;
      std::optional<TId> childId;
      Axis axis;
    };

    void makeState(Axis axis);
    State &ensureState() const;
    void setChildId(const BasicItemBuilder<TId> &itemBuilder, const std::optional<TId> &id);
    std::optional<TId> getChildId() const;
  };

  template <typename TId> class BasicVisibleListItems : public Flex
  {
   public:
    using Flex::Flex;
    using Widget::setModifier;
    using Widget::WidgetType;

    template <typename... tArgs>
    explicit BasicVisibleListItems(Widget &parent, Axis axis, tArgs... args)
        : Flex(parent, Scrollable::NO_SCROLL(), axis == Axis::Horizontal ? FlexFlow::HORIZONTAL() : FlexFlow::VERTICAL(),
               axis == Axis::Horizontal ? Expand::VERTICAL() : Expand::HORIZONTAL(), SizeVariant::FIT_CONTENT(), args...)
    {
      makeState(axis);
    }

    void bruteForceUpdate(int32_t parentExtend, int32_t scrollOffset, const ListModel<TId> &model, const BasicItemBuilder<TId> &itemBuilder,
                          const EmptyListPlaceholderBuilder &emptyListPlaceholderBuilder);
    int32_t getItemExtend() const;

    struct State
    {
      State(const BasicVisibleListItems &items, Axis axis)
          : handle(items.getHandle())
          , axis(axis)
      {
      }

      void setupChildren(int numItemsVisible) const;
      void bruteForceUpdate(int32_t parentExtend, int scrollOffset, const ListModel<TId> &model, const BasicItemBuilder<TId> &itemBuilder,
                            const EmptyListPlaceholderBuilder &emptyListPlaceholderBuilder) const;
      lv_obj_t *findItemFor(const std::optional<TId> &itemId) const;
      lv_obj_t *findItemForRecycling(const ListModel<TId> &model, size_t firstVisible, size_t numVisible) const;
      int32_t getItemExtend() const;
      lv_obj_t *handle = nullptr;
      Axis axis;
    };

    State &ensureState() const;
    void makeState(Axis axis);
  };

  template <typename TId> class BasicListPane : public Widget
  {
   public:
    using Widget::setModifier;
    using Widget::WidgetType;

    template <typename... tArgs>
    explicit BasicListPane(Widget &parent, Axis axis, tArgs... args)
        : Widget(parent, Padding { 0 }, Scrollable::NO_SCROLL(), LayoutType::none(), args...)
    {
      makeState(axis);
    }

    explicit BasicListPane(WidgetType *w)
        : Widget(w)
    {
    }

    bool shouldClearBeforeAutorunCompose() const override;
    void bruteForceUpdate(int32_t parentExtend, int32_t scrollOffset, const ListModel<TId> &model, const BasicItemBuilder<TId> &itemBuilder,
                          const EmptyListPlaceholderBuilder &emptyListPlaceholderBuilder);

    struct State
    {
      State(BasicListPane &pane, Axis axis)
          : handle(pane.getHandle())
          , visibleItems(pane, axis)
          , axis(axis)
      {
      }

      void bruteForceUpdate(int32_t parentExtend, int32_t scrollOffset, const ListModel<TId> &model, const BasicItemBuilder<TId> &itemBuilder,
                            const EmptyListPlaceholderBuilder &emptyListPlaceholderBuilder);

      lv_obj_t *handle = nullptr;
      BasicVisibleListItems<TId> visibleItems;
      Axis axis;
    };

    void makeState(Axis axis);
    State &ensureState();
  };

  template <typename TId> class BasicList : public Widget
  {
   public:
    using Widget::setModifier;
    using Widget::WidgetType;

    template <typename... tArgs>
    explicit BasicList(Widget &parent, Axis axis, tArgs... args)
        : Widget(parent, Scrollable::SCROLL(), std::forward<tArgs>(args)...)
    {
      makeState(axis);

      auto &state = ensureState();
      lv_obj_add_event_cb(getHandle(), onListChanged, LV_EVENT_SCROLL, &state);
      lv_obj_add_event_cb(getHandle(), onListChanged, LV_EVENT_SIZE_CHANGED, &state);
    }

    explicit BasicList(WidgetType *w)
        : Widget(w)
    {
    }

    struct State
    {
      State(Widget &list, Axis axis)
          : handle(list.getHandle())
          , pane(list, axis)
          , axis(axis)
      {
      }

      lv_obj_t *handle = nullptr;
      ListModel<TId> model;
      BasicListPane<TId> pane;
      Axis axis;

      BasicItemBuilder<TId> itemBuilder;
      EmptyListPlaceholderBuilder emptyListPlaceholderBuilder;

      void setModel(ListModel<TId> m);
      void scrollTo(const TId &id);

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

    struct PlaceHolder
    {
      BasicList *m_parent;

      template <typename CB> void operator<<(CB &&cb) const
      {
        m_parent->ensureState().setEmptyListPlaceholderBuilder(std::move(cb));
      }
    } emptyListPlaceholder { this };

    struct ScrollTo
    {
      BasicList *m_parent;

      template <typename CB> void operator<<(CB &&cb) const
      {
        m_parent->doAutorun([h = m_parent->getHandle(), cb = std::move(cb)] { BasicList(h).ensureState().scrollTo(cb()); });
      }
    } scrollTo { this };
  };

  class ListById : public BasicList<ItemId>
  {
   public:
    using BasicList<ItemId>::BasicList;

    struct ItemIds
    {
      ListById *m_parent;

      template <typename CB> void operator<<(CB &&cb) const
      {
        m_parent->doAutorun([h = m_parent->getHandle(), cb = std::move(cb)] { ListById(h).ensureState().setModel({ cb() }); });
      }
    } itemIds { this };

    struct ItemById
    {
      ListById *m_parent;

      template <typename CB> void operator<<(CB &&cb) const
      {
        m_parent->ensureState().setItemBuilder(std::move(cb));
      }
    } itemById { this };
  };

  class ListByIndex : public BasicList<size_t>
  {
   public:
    using BasicList<size_t>::BasicList;

    struct Size
    {
      ListByIndex *m_parent;

      template <typename CB> void operator<<(CB &&cb) const
      {
        m_parent->doAutorun([h = m_parent->getHandle(), cb = std::move(cb)] { ListByIndex(h).ensureState().setModel({ cb() }); });
      }
    } size { this };

    struct ItemByIndex
    {
      ListByIndex *m_parent;

      template <typename CB> void operator<<(CB &&cb) const
      {
        m_parent->ensureState().setItemBuilder(std::move(cb));
      }
    } itemByIndex { this };
  };
}

#define LIST_BY_ID(...) it.add(std::move(Compose::ListById(it __VA_OPT__(, __VA_ARGS__)))) << [=](Compose::ListById && it)
#define LIST_ITEM_IDS() it.itemIds << [=]()
#define LIST_ITEM_BY_ID it.itemById << [=]

#define LIST_BY_INDEX(...) it.add(std::move(Compose::ListByIndex(it __VA_OPT__(, __VA_ARGS__)))) << [=](Compose::ListByIndex && it)
#define LIST_SIZE() it.size << [=]()
#define LIST_ITEM_BY_INDEX it.itemByIndex << [=]

#define LIST_SCROLL_TO() it.scrollTo << [=]()
#define EMPTY_LIST_PLACEHOLDER() it.emptyListPlaceholder << [=](Compose::Widget & it)
