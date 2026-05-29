#include "compose/widgets/container/List.h"

namespace Compose
{
  ListItemContainer::ListItemContainer(WidgetType *w)
      : Widget(w)
  {
  }

  ListItemContainer::State::State(const ListItemContainer &container, Axis axis)
      : handle(container.getHandle())
      , axis(axis)
  {
  }

  void ListItemContainer::State::setChildId(const ItemBuilder &itemBuilder, const std::optional<ItemId> &id)
  {
    if(std::exchange(childId, id) != id)
    {
      ListItemContainer widget(handle);
      lv_obj_clean(handle);
      if(childId.has_value())
      {
        itemBuilder(widget, childId.value());
      }
    }
  }

  void ListItemContainer::makeState(Axis axis)
  {
    static constexpr auto c_key = "ListPaneContainerState";
    ensureDataForKeyExistsOwning<State>(c_key, [this, axis] { return new State(*this, axis); });
  }

  ListItemContainer::State &ListItemContainer::ensureState() const
  {
    static constexpr auto c_key = "ListPaneContainerState";
    return *getData<State>(c_key);
  }

  void ListItemContainer::setChildId(const ItemBuilder &itemBuilder, const std::optional<ItemId> &id)
  {
    ensureState().setChildId(itemBuilder, id);
  }

  std::optional<ItemId> ListItemContainer::getChildId() const
  {
    return ensureState().childId;
  }

  void VisibleListItems::bruteForceUpdate(int32_t parentExtend, int32_t scrollOffset, const std::vector<ItemId> &idMap, const ItemBuilder &itemBuilder,
                                          const EmptyListPlaceholderBuilder &emptyListPlaceholderBuilder)
  {
    ensureState().bruteForceUpdate(parentExtend, scrollOffset, idMap, itemBuilder, emptyListPlaceholderBuilder);
  }

  int32_t VisibleListItems::getItemExtend() const
  {
    return ensureState().getItemExtend();
  }

  VisibleListItems::State::State(const VisibleListItems &items, Axis axis)
      : handle(items.getHandle())
      , axis(axis)
  {
  }

  void VisibleListItems::State::setupChildren(int numItemsVisible) const
  {
    VisibleListItems self(handle);

    while(lv_obj_get_child_count(handle) > numItemsVisible)
    {
      auto child = lv_obj_get_child(handle, 0);
      lv_obj_delete(child);
    }

    while(lv_obj_get_child_count(handle) < numItemsVisible)
    {
      ListItemContainer container(self, axis);
    }
  }

  void VisibleListItems::State::bruteForceUpdate(int32_t parentExtend, int scrollOffset, const std::vector<ItemId> &idMap, const ItemBuilder &itemBuilder,
                                                 const EmptyListPlaceholderBuilder &emptyListPlaceholderBuilder) const
  {
    VisibleListItems self(handle);

    if(idMap.empty() && emptyListPlaceholderBuilder)
    {
      setupChildren(1);
      auto containerHandle = lv_obj_get_child(handle, 0);
      ListItemContainer container(containerHandle);
      container.setChildId(itemBuilder, std::nullopt);
      emptyListPlaceholderBuilder(container);
      return;
    }

    if(itemBuilder)
    {
      auto itemExtend = getItemExtend();
      auto numItemsVisible = std::min<int>(idMap.size(), parentExtend / itemExtend + 2);
      setupChildren(numItemsVisible);

      auto numChildren = lv_obj_get_child_count(handle);
      auto firstItemIdx = scrollOffset / itemExtend;
      auto scrollPos = itemExtend * (scrollOffset / itemExtend);

      if(axis == Axis::Horizontal)
        lv_obj_set_pos(handle, scrollPos, 0);
      else
        lv_obj_set_pos(handle, 0, scrollPos);

      auto range = idMap | std::views::drop(firstItemIdx) | std::views::take(numItemsVisible);

      for(auto itemIdx = firstItemIdx; itemIdx < firstItemIdx + numItemsVisible; ++itemIdx)
      {
        auto childIdx = itemIdx - firstItemIdx;
        if(childIdx < numChildren)
        {
          auto itemId = itemIdx < idMap.size() ? std::optional { idMap[itemIdx] } : std::nullopt;

          if(auto matchingChild = findItemFor(itemId))
          {
            lv_obj_move_to_index(matchingChild, childIdx);
          }
          else if(auto recycle = findItemForRecycling(range))
          {
            ListItemContainer w(recycle);
            w.setChildId(itemBuilder, itemId);
            lv_obj_move_to_index(recycle, childIdx);
          }
          else
          {
            throw std::runtime_error("Failed to find or recycle item for List");
          }
        }
      }
    }
  }

  lv_obj_t *VisibleListItems::State::findItemFor(const std::optional<ItemId> &itemId) const
  {
    auto numChildren = lv_obj_get_child_count(handle);
    for(auto idx = 0; idx < numChildren; idx++)
    {
      auto childHandle = lv_obj_get_child(handle, idx);
      ListItemContainer c(childHandle);
      auto childId = c.getChildId();
      if(childId == itemId)
        return childHandle;
    }
    return nullptr;
  }

  int32_t VisibleListItems::State::getItemExtend() const
  {
    auto numChildren = lv_obj_get_child_count(handle);
    auto itemExtend = 1;

    for(int i = 0; i < numChildren; i++)
    {
      auto child = lv_obj_get_child(handle, i);
      if(axis == Axis::Horizontal)
        itemExtend = std::max(itemExtend, lv_obj_get_width(child));
      else
        itemExtend = std::max(itemExtend, lv_obj_get_height(child));
    }

    return itemExtend;
  }

  lv_obj_t *VisibleListItems::State::findItemForRecycling(const auto &range) const
  {
    auto numChildren = lv_obj_get_child_count(handle);
    for(auto idx = 0; idx < numChildren; idx++)
    {
      auto childHandle = lv_obj_get_child(handle, idx);
      ListItemContainer c(childHandle);
      auto childId = c.getChildId();
      if(!childId.has_value() || std::ranges::find(range, childId) == std::ranges::end(range))
        return childHandle;
    }
    return nullptr;
  }

  VisibleListItems::State &VisibleListItems::ensureState() const
  {
    static constexpr auto c_key = "VisibleListItemsState";
    return *(getData<State>(c_key));
  }

  void VisibleListItems::makeState(Axis axis)
  {
    static constexpr auto c_key = "VisibleListItemsState";
    ensureDataForKeyExistsOwning<State>(c_key, [this, axis] { return new State(*this, axis); });
  }

  ListPane::ListPane(WidgetType *w)
      : Widget(w)
  {
  }

  bool ListPane::shouldClearBeforeAutorunCompose() const
  {
    return false;
  }

  void ListPane::bruteForceUpdate(int32_t parentExtend, int32_t scrollOffset, const std::vector<ItemId> &idMap, const ItemBuilder &itemBuilder,
                                  const EmptyListPlaceholderBuilder &emptyListPlaceholderBuilder)
  {
    ensureState().bruteForceUpdate(parentExtend, scrollOffset, idMap, itemBuilder, emptyListPlaceholderBuilder);
  }

  ListPane::State::State(ListPane &pane, Axis axis)
      : handle(pane.getHandle())
      , visibleItems(pane, axis)
      , axis(axis)
  {
  }

  void ListPane::State::bruteForceUpdate(int32_t parentExtend, int32_t scrollOffset, const std::vector<ItemId> &idMap, const ItemBuilder &itemBuilder,
                                         const EmptyListPlaceholderBuilder &emptyListPlaceholderBuilder)
  {
    const auto itemExtend = visibleItems.getItemExtend();
    const auto contentExtend = std::max<int32_t>(parentExtend, idMap.size() * itemExtend);
    const auto maxScroll = std::max(0, contentExtend - parentExtend);

    if(axis == Axis::Horizontal)
      lv_obj_set_width(handle, contentExtend);
    else
      lv_obj_set_height(handle, contentExtend);

    visibleItems.bruteForceUpdate(parentExtend, std::clamp(scrollOffset, 0, maxScroll), idMap, itemBuilder, emptyListPlaceholderBuilder);
  }

  void ListPane::makeState(Axis axis)
  {
    static constexpr auto c_key = "ListPaneState";
    ensureDataForKeyExistsOwning<State>(c_key, [this, axis] { return new State(*this, axis); });
  }

  ListPane::State &ListPane::ensureState()
  {
    static constexpr auto c_key = "ListPaneState";
    return *getData<State>(c_key);
  }

  List::List(WidgetType *w)
      : Widget(w)
  {
  }

  bool List::shouldClearBeforeAutorunCompose() const
  {
    return false;
  }

  void List::onListChanged(lv_event_t *e)
  {
    static_cast<State *>(lv_event_get_user_data(e))->bruteForceUpdate();
  }

  List::State::State(Widget &list, Axis axis)
      : handle(list.getHandle())
      , pane(list, axis)
      , axis(axis)
  {
  }

  void List::State::setIdMap(std::vector<ItemId> map)
  {
    idMap = std::move(map);
    bruteForceUpdate();
  }

  void List::State::scrollTo(const ItemId &id)
  {
    auto it = std::find(idMap.begin(), idMap.end(), id);
    if(it != idMap.end())
    {
      auto modelIndex = std::distance(idMap.begin(), it);

      const auto stride = this->pane.ensureState().visibleItems.getItemExtend();
      const auto viewportExtent = axis == Axis::Horizontal ? lv_obj_get_content_width(handle) : lv_obj_get_content_height(handle);

      const auto rowStart = modelIndex * stride;
      const auto rowEnd = rowStart + stride;

      auto vpStart = axis == Axis::Horizontal ? lv_obj_get_scroll_left(handle) : lv_obj_get_scroll_top(handle);
      auto vpEnd = vpStart + viewportExtent;

      if(rowStart < vpStart)
      {
        if(axis == Axis::Horizontal)
          lv_obj_scroll_to_x(handle, rowStart, LV_ANIM_OFF);
        else
          lv_obj_scroll_to_y(handle, rowStart, LV_ANIM_OFF);
      }
      else if(rowEnd > vpEnd)
      {
        if(axis == Axis::Horizontal)
          lv_obj_scroll_by(handle, vpEnd - rowEnd, 0, LV_ANIM_OFF);
        else
          lv_obj_scroll_by(handle, 0, vpEnd - rowEnd, LV_ANIM_OFF);
      }
    }
  }

  void List::State::bruteForceUpdate()
  {
    if(axis == Axis::Horizontal)
      pane.bruteForceUpdate(lv_obj_get_content_width(handle), lv_obj_get_scroll_left(handle), idMap, itemBuilder, emptyListPlaceholderBuilder);
    else
      pane.bruteForceUpdate(lv_obj_get_content_height(handle), lv_obj_get_scroll_top(handle), idMap, itemBuilder, emptyListPlaceholderBuilder);
  }

  void List::makeState(Axis axis)
  {
    static constexpr auto c_key = "ListState";
    ensureDataForKeyExistsOwning<State>(c_key, [this, axis] { return new State(*this, axis); });
  }

  List::State &List::ensureState()
  {
    static constexpr auto c_key = "ListState";
    return *getData<State>(c_key);
  }
}