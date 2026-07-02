#include "compose/widgets/container/VirtualizedList.h"

#include "src/core/lv_obj_event.h"

#include <algorithm>
#include <vector>
#include <nltools/Assert.h>

namespace Compose
{
  namespace
  {
    constexpr int c_defaultOverscanItems = 2;
    constexpr int c_visibleOverscanSides = 2;
    constexpr int c_unmappedFirstIndex = -1;
    constexpr int c_defaultItemExtent = 24;
    constexpr int c_insertIndexAfterTopSpacer = 1;
    constexpr int c_insertIndexBeforeBottomSpacerOffset = 2;
  }

  struct VirtualizedList::State
  {
    Axis axis;
    lv_obj_t *handle = nullptr;
    ItemBuilder itemBuilder;
    lv_obj_t *topSpacer = nullptr;
    lv_obj_t *bottomSpacer = nullptr;
    std::vector<lv_obj_t *> rows;
    lv_event_dsc_t *scrollHandler = nullptr;
    lv_event_dsc_t *sizeHandler = nullptr;
    int itemCount = 0;
    int overscan = c_defaultOverscanItems;
    int firstMappedIndex = c_unmappedFirstIndex;
    int itemExtent = c_defaultItemExtent;
    bool isRefreshing = false;

    explicit State(lv_obj_t *h, Axis a)
        : axis(a)
        , handle(h)
    {
    }

    ~State()
    {
      if(handle && lv_obj_is_valid(handle))
      {
        if(scrollHandler)
          lv_obj_remove_event_dsc(handle, scrollHandler);

        if(sizeHandler)
          lv_obj_remove_event_dsc(handle, sizeHandler);
      }
    }

    static void onListChanged(lv_event_t *e)
    {
      Reactive::Deferrer deferrer;
      if(auto *self = static_cast<State *>(lv_event_get_user_data(e)))
      {
        auto *currentTarget = static_cast<lv_obj_t *>(lv_event_get_current_target(e));
        self->refresh(currentTarget);
      }
    }

    [[nodiscard]] bool isValidListHandle(lv_obj_t *currentHandle)
    {
      if(currentHandle && lv_obj_is_valid(currentHandle))
      {
        handle = currentHandle;
        return true;
      }

      if(currentHandle != nullptr)
      {
        invalidateStructure();
        firstMappedIndex = c_unmappedFirstIndex;
      }

      return false;
    }

    void ensureHandlers()
    {
      nltools_detailedAssertAlways(handle && lv_obj_is_valid(handle),
                                   "VirtualList handle invalid while registering handlers");

      if(scrollHandler == nullptr)
        scrollHandler = lv_obj_add_event_cb(handle, onListChanged, LV_EVENT_SCROLL, this);

      if(sizeHandler == nullptr)
        sizeHandler = lv_obj_add_event_cb(handle, onListChanged, LV_EVENT_SIZE_CHANGED, this);
    }

    [[nodiscard]] int currentVisibleItems(lv_obj_t *targetHandle, int itemStride) const
    {
      const auto viewportExtent
          = axis == Axis::Vertical ? lv_obj_get_content_height(targetHandle) : lv_obj_get_content_width(targetHandle);
      const auto stride = std::max(1, itemStride);
      return std::max(1, (std::max(1, viewportExtent) + stride - 1) / stride);
    }

    [[nodiscard]] int measureStride(lv_obj_t *targetHandle) const
    {
      const auto rowGap = axis == Axis::Vertical ? lv_obj_get_style_pad_row(targetHandle, LV_PART_MAIN)
                                                 : lv_obj_get_style_pad_column(targetHandle, LV_PART_MAIN);
      const auto measured = itemExtent + rowGap;
      return std::max(1, measured);
    }

    void invalidateStructure()
    {
      topSpacer = nullptr;
      bottomSpacer = nullptr;
      rows.clear();
    }

    [[nodiscard]] lv_obj_t *createStyledWidget(lv_obj_t *listHandle) const
    {
      Widget w(lv_obj_create(listHandle));
      w.applyDefaultStyle(w.getHandle());
      w.setModifier(BackgroundColor::TRANSPARENT());
      w.setDefaultWidthAndHeightAccordingToParent();
      lv_obj_set_style_flex_grow(w.getHandle(), 0, LV_PART_MAIN);
      return w.getHandle();
    }

    void rebuildStructure(lv_obj_t *listHandle, int renderedItems)
    {
      lv_obj_clean(listHandle);
      invalidateStructure();

      topSpacer = createStyledWidget(listHandle);
      rows.reserve(renderedItems);

      for(auto i = 0; i < renderedItems; i++)
        rows.push_back(createStyledWidget(listHandle));

      bottomSpacer = createStyledWidget(listHandle);
    }

    [[nodiscard]] bool hasValidStructure(lv_obj_t *listHandle, int renderedItems) const
    {
      const auto spacersValid = topSpacer && bottomSpacer && lv_obj_is_valid(topSpacer) && lv_obj_is_valid(bottomSpacer)
          && lv_obj_get_parent(topSpacer) == listHandle && lv_obj_get_parent(bottomSpacer) == listHandle;

      const auto rowsValid = static_cast<int>(rows.size()) == renderedItems
          && std::all_of(rows.begin(), rows.end(), [listHandle](const auto row) {
                               return row && lv_obj_is_valid(row) && lv_obj_get_parent(row) == listHandle;
                             });

      return spacersValid && rowsValid;
    }

    [[nodiscard]] int currentScrollOffset(lv_obj_t *listHandle) const
    {
      return std::max(0,
                      axis == Axis::Vertical ? lv_obj_get_scroll_top(listHandle) : lv_obj_get_scroll_left(listHandle));
    }

    [[nodiscard]] int firstVisibleItem(int scrollOffset, int itemStride, int renderedItems) const
    {
      const auto unclampedFirst = scrollOffset / std::max(1, itemStride);
      const auto maxFirst = std::max(0, itemCount - renderedItems);
      return std::min(unclampedFirst, maxFirst);
    }

    void setSpacerExtent(lv_obj_t *spacer, int extent) const
    {
      if(axis == Axis::Vertical)
        lv_obj_set_height(spacer, extent);
      else
        lv_obj_set_width(spacer, extent);
    }

    void rebuildRowContent(lv_obj_t *rowHandle, int modelIndex) const
    {
      nltools_detailedAssertAlways(rowHandle && lv_obj_is_valid(rowHandle), "VirtualList row handle invalid");
      Widget row(rowHandle);

      if(auto *storage = row.getUserDataStorage())
        storage->entries.erase(c_computationsKey);

      row.clear();
      itemBuilder(row, modelIndex);
    }

    void remapAllRows(int first, int renderedItems) const
    {
      for(auto slot = 0; slot < renderedItems; slot++)
        rebuildRowContent(rows[slot], first + slot);
    }

    [[nodiscard]] int renderedItemCount(lv_obj_t *listHandle, int itemStride) const
    {
      const auto visibleItems = currentVisibleItems(listHandle, itemStride);
      const auto visibleAndOverscan = visibleItems + c_visibleOverscanSides * overscan;
      return std::min(itemCount, visibleAndOverscan);
    }

    void clearListItems(lv_obj_t *listHandle)
    {
      lv_obj_clean(listHandle);
      invalidateStructure();
      firstMappedIndex = 0;
    }

    void ensureValidStructure(lv_obj_t *listHandle, int renderedItems, bool &forceRebuildAll)
    {
      if(!hasValidStructure(listHandle, renderedItems))
      {
        rebuildStructure(listHandle, renderedItems);
        forceRebuildAll = true;
      }

      if(forceRebuildAll)
        firstMappedIndex = c_unmappedFirstIndex;
    }

    [[nodiscard]] int mapRowsToFirstVisibleItem(int first, int renderedItems, bool forceRebuildAll)
    {
      if(firstMappedIndex == c_unmappedFirstIndex || forceRebuildAll)
      {
        remapAllRows(first, renderedItems);
        return first;
      }

      return remapShiftedRows(firstMappedIndex, first, renderedItems);
    }

    void updateVirtualSpacing(int mappedFirst, int renderedItems, int itemStride) const
    {
      const auto topVirtualSpace = mappedFirst * itemStride;
      const auto tailCount = std::max(0, itemCount - (mappedFirst + renderedItems));
      const auto bottomVirtualSpace = tailCount * itemStride;
      setSpacerExtent(topSpacer, topVirtualSpace);
      setSpacerExtent(bottomSpacer, bottomVirtualSpace);
    }

    [[nodiscard]] static bool isDescendantOf(lv_obj_t *node, lv_obj_t *candidateParent)
    {
      auto *current = node;
      while(current)
      {
        if(current == candidateParent)
          return true;
        current = lv_obj_get_parent(current);
      }

      return false;
    }

    [[nodiscard]] int findProtectedRowIndex() const
    {
      if(auto *activeObj = lv_indev_get_active_obj())
        if(lv_obj_is_valid(activeObj) && handle && lv_obj_is_valid(handle) && isDescendantOf(activeObj, handle))
          for(auto i = 0; i < static_cast<int>(rows.size()); i++)
            if(isDescendantOf(activeObj, rows[i]))
              return i;

      return -1;
    }

    [[nodiscard]] int clampDeltaWithProtectedRow(int delta, int renderedItems) const
    {
      if(const auto protectedRow = findProtectedRowIndex(); protectedRow >= 0)
      {
        if(delta < 0)
          delta = -std::min(-delta, renderedItems - 1 - protectedRow);
        if(delta > 0)
          delta = std::min(delta, protectedRow);
      }

      return delta;
    }

    void recycleRowsForward(int appliedFirst, int renderedItems, int delta)
    {
      for(auto step = 0; step < delta; step++)
      {
        auto *recycled = rows.front();
        rows.erase(rows.begin());
        rows.push_back(recycled);
        nltools_detailedAssertAlways(handle && lv_obj_is_valid(handle), "VirtualList handle invalid during remap");
        lv_obj_move_to_index(
            recycled, static_cast<int32_t>(lv_obj_get_child_count(handle)) - c_insertIndexBeforeBottomSpacerOffset);
        const auto modelIndex = appliedFirst + renderedItems - delta + step;
        rebuildRowContent(recycled, modelIndex);
      }
    }

    void recycleRowsBackward(int appliedFirst, int backwardSteps)
    {
      for(auto step = 0; step < backwardSteps; step++)
      {
        auto *recycled = rows.back();
        rows.pop_back();
        rows.insert(rows.begin(), recycled);
        nltools_detailedAssertAlways(handle && lv_obj_is_valid(handle), "VirtualList handle invalid during remap");
        lv_obj_move_to_index(recycled, c_insertIndexAfterTopSpacer);
        const auto modelIndex = appliedFirst + backwardSteps - step - 1;
        rebuildRowContent(recycled, modelIndex);
      }
    }

    void remapVisibleWindow(int appliedFirst, int renderedItems, int delta)
    {
      if(delta > 0)
      {
        recycleRowsForward(appliedFirst, renderedItems, delta);
        return;
      }

      nltools_detailedAssertAlways(delta < 0, "VirtualList remap expected negative delta");
      recycleRowsBackward(appliedFirst, -delta);
    }

    [[nodiscard]] int remapShiftedRows(int oldFirst, int newFirst, int renderedItems)
    {
      const auto delta = clampDeltaWithProtectedRow(newFirst - oldFirst, renderedItems);
      auto mappedFirst = oldFirst;
      if(delta != 0)
      {
        mappedFirst = oldFirst + delta;
        if(std::abs(delta) >= renderedItems)
          remapAllRows(mappedFirst, renderedItems);
        else
          remapVisibleWindow(mappedFirst, renderedItems, delta);
      }

      return mappedFirst;
    }

    struct RenderWindow
    {
      int itemStride = 0;
      int renderedItems = 0;
      int first = 0;
    };

    [[nodiscard]] RenderWindow buildRenderWindow(lv_obj_t *listHandle) const
    {
      const auto itemStride = measureStride(listHandle);
      const auto renderedItems = renderedItemCount(listHandle, itemStride);
      const auto scrollOffset = currentScrollOffset(listHandle);
      const auto first = firstVisibleItem(scrollOffset, itemStride, renderedItems);
      return { itemStride, renderedItems, first };
    }

    void refreshRowsAndSpacing(lv_obj_t *listHandle, bool forceRebuildAll, const RenderWindow &window)
    {
      ensureValidStructure(listHandle, window.renderedItems, forceRebuildAll);
      const auto mappedFirst = mapRowsToFirstVisibleItem(window.first, window.renderedItems, forceRebuildAll);
      updateVirtualSpacing(mappedFirst, window.renderedItems, window.itemStride);
      firstMappedIndex = mappedFirst;
    }

    void updateListLayout(lv_obj_t *listHandle) const
    {
      lv_obj_update_layout(listHandle);
      lv_obj_invalidate(listHandle);
    }

    [[nodiscard]] lv_obj_t *currentValidHandle() const
    {
      auto *listHandle = handle;
      nltools_detailedAssertAlways(listHandle && lv_obj_is_valid(listHandle),
                                   "VirtualList handle invalid during refresh");
      return listHandle;
    }

    [[nodiscard]] bool hasNoItems(lv_obj_t *listHandle)
    {
      if(itemCount == 0)
      {
        clearListItems(listHandle);
        return true;
      }

      return false;
    }

    void refreshMappedWindow(lv_obj_t *listHandle, bool forceRebuildAll)
    {
      const auto window = buildRenderWindow(listHandle);
      if(!forceRebuildAll && firstMappedIndex == window.first)
        return;
      refreshRowsAndSpacing(listHandle, forceRebuildAll, window);
      updateListLayout(listHandle);
    }

    void refreshCurrentList(bool forceRebuildAll)
    {
      ensureHandlers();
      auto *listHandle = currentValidHandle();
      if(hasNoItems(listHandle))
        return;
      refreshMappedWindow(listHandle, forceRebuildAll);
    }

    struct RefreshGuard
    {
      bool &refreshing;
      ~RefreshGuard()
      {
        refreshing = false;
      }
    };

    void refresh(lv_obj_t *currentHandle, bool forceRebuildAll = false)
    {
      if(!isRefreshing && isValidListHandle(currentHandle) && itemBuilder)
      {
        isRefreshing = true;
        RefreshGuard guard { isRefreshing };
        refreshCurrentList(forceRebuildAll);
      }
    }
  };

  VirtualizedList::VirtualizedList(WidgetType *w, Axis axis)
      : Widget(w)
      , m_axis(axis)
  {
    ensureState();
  }

  void VirtualizedList::clear()
  {
    if(auto *state = getData<State>(c_virtualListStateKey))
    {
      state->topSpacer = nullptr;
      state->bottomSpacer = nullptr;
      state->rows.clear();
      state->firstMappedIndex = c_unmappedFirstIndex;
      state->isRefreshing = false;
      state->itemBuilder = nullptr;
    }

    lv_obj_clean(getHandle());
  }

  void VirtualizedList::setModifier(ItemCount count) const
  {
    auto &state = ensureState();
    const auto newCount = std::max(0, count.it);
    const auto hasChanged = state.itemCount != newCount;
    state.itemCount = newCount;
    state.refresh(getHandle(), hasChanged);
  }

  void VirtualizedList::setOverscan(int overscanItems) const
  {
    auto &state = ensureState();
    state.overscan = std::max(0, overscanItems);
    state.refresh(getHandle());
  }

  void VirtualizedList::setItemBuilder(ItemBuilder cb) const
  {
    auto &state = ensureState();
    state.itemBuilder = std::move(cb);
    state.refresh(getHandle(), true);
  }

  void VirtualizedList::setItemExtent(int extent) const
  {
    auto &state = ensureState();
    state.itemExtent = std::max(1, extent);
    state.refresh(getHandle(), true);
  }

  int VirtualizedList::getItemExtent() const
  {
    return ensureState().itemExtent;
  }

  VirtualizedList::State &VirtualizedList::ensureState() const
  {
    return ensureDataForKeyExistsOwning<State>(
        c_virtualListStateKey, [handle = getHandle(), axis = m_axis] { return new State(handle, axis); });
  }
}
