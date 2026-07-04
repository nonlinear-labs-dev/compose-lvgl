#include "compose/widgets/container/List.h"

namespace Compose
{
  template <typename TId> void BasicListItemContainer<TId>::State::setChildId(const BasicItemBuilder<TId> &itemBuilder, const std::optional<TId> &id)
  {
    if(std::exchange(childId, id) != id)
    {
      BasicListItemContainer widget(handle);
      lv_obj_clean(handle);
      if(childId.has_value())
      {
        itemBuilder(widget, childId.value());
      }
    }
  }

  template <typename TId> void BasicListItemContainer<TId>::makeState(Axis axis)
  {
    static constexpr auto c_key = "ListPaneContainerState";
    this->template ensureDataForKeyExistsOwning<State>(c_key, [this, axis] { return new State(*this, axis); });
  }

  template <typename TId> typename BasicListItemContainer<TId>::State &BasicListItemContainer<TId>::ensureState() const
  {
    static constexpr auto c_key = "ListPaneContainerState";
    return *(this->template getData<State>(c_key));
  }

  template <typename TId> void BasicListItemContainer<TId>::setChildId(const BasicItemBuilder<TId> &itemBuilder, const std::optional<TId> &id)
  {
    ensureState().setChildId(itemBuilder, id);
  }

  template <typename TId> std::optional<TId> BasicListItemContainer<TId>::getChildId() const
  {
    return ensureState().childId;
  }

  template <typename TId>
  void BasicVisibleListItems<TId>::bruteForceUpdate(int32_t parentExtend, int32_t scrollOffset, const ListModel<TId> &model,
                                                    const BasicItemBuilder<TId> &itemBuilder, const EmptyListPlaceholderBuilder &emptyListPlaceholderBuilder)
  {
    ensureState().bruteForceUpdate(parentExtend, scrollOffset, model, itemBuilder, emptyListPlaceholderBuilder);
  }

  template <typename TId> int32_t BasicVisibleListItems<TId>::getItemExtend() const
  {
    return ensureState().getItemExtend();
  }

  template <typename TId> void BasicVisibleListItems<TId>::State::setupChildren(int numItemsVisible) const
  {
    BasicVisibleListItems self(handle);

    while(lv_obj_get_child_count(handle) > numItemsVisible)
    {
      auto child = lv_obj_get_child(handle, 0);
      lv_obj_delete(child);
    }

    while(lv_obj_get_child_count(handle) < numItemsVisible)
    {
      BasicListItemContainer<TId> container(self, axis);
    }
  }

  template <typename TId>
  void BasicVisibleListItems<TId>::State::bruteForceUpdate(int32_t parentExtend, int scrollOffset, const ListModel<TId> &model,
                                                           const BasicItemBuilder<TId> &itemBuilder,
                                                           const EmptyListPlaceholderBuilder &emptyListPlaceholderBuilder) const
  {
    if(model.size() == 0 && emptyListPlaceholderBuilder)
    {
      setupChildren(1);
      auto containerHandle = lv_obj_get_child(handle, 0);
      BasicListItemContainer<TId> container(containerHandle);
      container.setChildId(itemBuilder, std::nullopt);
      if(lv_obj_get_child_count(containerHandle) == 0)
        emptyListPlaceholderBuilder(container);
      return;
    }

    if(itemBuilder)
    {
      auto itemExtend = getItemExtend();
      auto numItemsVisible = std::min<int>(model.size(), parentExtend / itemExtend + 2);
      setupChildren(numItemsVisible);

      auto numChildren = lv_obj_get_child_count(handle);
      auto firstItemIdx = scrollOffset / itemExtend;
      auto scrollPos = itemExtend * (scrollOffset / itemExtend);

      if(axis == Axis::Horizontal)
        lv_obj_set_pos(handle, scrollPos, 0);
      else
        lv_obj_set_pos(handle, 0, scrollPos);

      for(auto itemIdx = firstItemIdx; itemIdx < firstItemIdx + numItemsVisible; ++itemIdx)
      {
        auto childIdx = itemIdx - firstItemIdx;
        if(childIdx < numChildren)
        {
          auto itemId = itemIdx < model.size() ? std::optional { model.at(itemIdx) } : std::nullopt;

          if(auto matchingChild = findItemFor(itemId))
          {
            lv_obj_move_to_index(matchingChild, childIdx);
          }
          else if(auto recycle = findItemForRecycling(model, firstItemIdx, numItemsVisible))
          {
            BasicListItemContainer<TId> w(recycle);
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

  template <typename TId> lv_obj_t *BasicVisibleListItems<TId>::State::findItemFor(const std::optional<TId> &itemId) const
  {
    auto numChildren = lv_obj_get_child_count(handle);
    for(auto idx = 0; idx < numChildren; idx++)
    {
      auto childHandle = lv_obj_get_child(handle, idx);
      BasicListItemContainer<TId> c(childHandle);
      auto childId = c.getChildId();
      if(childId == itemId)
        return childHandle;
    }
    return nullptr;
  }

  template <typename TId> int32_t BasicVisibleListItems<TId>::State::getItemExtend() const
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

  template <typename TId>
  lv_obj_t *BasicVisibleListItems<TId>::State::findItemForRecycling(const ListModel<TId> &model, size_t firstVisible, size_t numVisible) const
  {
    auto isVisible = [&](const TId &id) {
      for(auto i = firstVisible; i < firstVisible + numVisible && i < model.size(); i++)
        if(model.at(i) == id)
          return true;
      return false;
    };

    auto numChildren = lv_obj_get_child_count(handle);
    for(auto idx = 0; idx < numChildren; idx++)
    {
      auto childHandle = lv_obj_get_child(handle, idx);
      BasicListItemContainer<TId> c(childHandle);
      auto childId = c.getChildId();
      if(!childId.has_value() || !isVisible(childId.value()))
        return childHandle;
    }
    return nullptr;
  }

  template <typename TId> typename BasicVisibleListItems<TId>::State &BasicVisibleListItems<TId>::ensureState() const
  {
    static constexpr auto c_key = "VisibleListItemsState";
    return *(this->template getData<State>(c_key));
  }

  template <typename TId> void BasicVisibleListItems<TId>::makeState(Axis axis)
  {
    static constexpr auto c_key = "VisibleListItemsState";
    this->template ensureDataForKeyExistsOwning<State>(c_key, [this, axis] { return new State(*this, axis); });
  }

  template <typename TId> bool BasicListPane<TId>::shouldClearBeforeAutorunCompose() const
  {
    return false;
  }

  template <typename TId>
  void BasicListPane<TId>::bruteForceUpdate(int32_t parentExtend, int32_t scrollOffset, const ListModel<TId> &model, const BasicItemBuilder<TId> &itemBuilder,
                                            const EmptyListPlaceholderBuilder &emptyListPlaceholderBuilder)
  {
    ensureState().bruteForceUpdate(parentExtend, scrollOffset, model, itemBuilder, emptyListPlaceholderBuilder);
  }

  template <typename TId>
  void BasicListPane<TId>::State::bruteForceUpdate(int32_t parentExtend, int32_t scrollOffset, const ListModel<TId> &model,
                                                   const BasicItemBuilder<TId> &itemBuilder, const EmptyListPlaceholderBuilder &emptyListPlaceholderBuilder)
  {
    const auto itemExtend = visibleItems.getItemExtend();
    const auto contentExtend = std::max<int32_t>(parentExtend, model.size() * itemExtend);
    const auto maxScroll = std::max(0, contentExtend - parentExtend);

    if(axis == Axis::Horizontal)
      lv_obj_set_width(handle, contentExtend);
    else
      lv_obj_set_height(handle, contentExtend);

    visibleItems.bruteForceUpdate(parentExtend, std::clamp(scrollOffset, 0, maxScroll), model, itemBuilder, emptyListPlaceholderBuilder);
  }

  template <typename TId> void BasicListPane<TId>::makeState(Axis axis)
  {
    static constexpr auto c_key = "ListPaneState";
    this->template ensureDataForKeyExistsOwning<State>(c_key, [this, axis] { return new State(*this, axis); });
  }

  template <typename TId> typename BasicListPane<TId>::State &BasicListPane<TId>::ensureState()
  {
    static constexpr auto c_key = "ListPaneState";
    return *(this->template getData<State>(c_key));
  }

  template <typename TId> bool BasicList<TId>::shouldClearBeforeAutorunCompose() const
  {
    return false;
  }

  template <typename TId> void BasicList<TId>::onListChanged(lv_event_t *e)
  {
    static_cast<State *>(lv_event_get_user_data(e))->bruteForceUpdate();
  }

  template <typename TId> void BasicList<TId>::State::setModel(ListModel<TId> m)
  {
    model = std::move(m);
    bruteForceUpdate();
  }

  template <typename TId> void BasicList<TId>::State::scrollTo(const TId &id)
  {
    if(auto modelIndex = model.indexOf(id))
    {
      const auto stride = this->pane.ensureState().visibleItems.getItemExtend();
      const auto viewportExtent = axis == Axis::Horizontal ? lv_obj_get_content_width(handle) : lv_obj_get_content_height(handle);

      const auto rowStart = static_cast<int32_t>(*modelIndex) * stride;
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

  template <typename TId> void BasicList<TId>::State::bruteForceUpdate()
  {
    if(axis == Axis::Horizontal)
      pane.bruteForceUpdate(lv_obj_get_content_width(handle), lv_obj_get_scroll_left(handle), model, itemBuilder, emptyListPlaceholderBuilder);
    else
      pane.bruteForceUpdate(lv_obj_get_content_height(handle), lv_obj_get_scroll_top(handle), model, itemBuilder, emptyListPlaceholderBuilder);
  }

  template <typename TId> void BasicList<TId>::makeState(Axis axis)
  {
    static constexpr auto c_key = "ListState";
    this->template ensureDataForKeyExistsOwning<State>(c_key, [this, axis] { return new State(*this, axis); });
  }

  template <typename TId> typename BasicList<TId>::State &BasicList<TId>::ensureState()
  {
    static constexpr auto c_key = "ListState";
    return *(this->template getData<State>(c_key));
  }

  template class BasicListItemContainer<ItemId>;
  template class BasicListItemContainer<size_t>;
  template class BasicVisibleListItems<ItemId>;
  template class BasicVisibleListItems<size_t>;
  template class BasicListPane<ItemId>;
  template class BasicListPane<size_t>;
  template class BasicList<ItemId>;
  template class BasicList<size_t>;
}
