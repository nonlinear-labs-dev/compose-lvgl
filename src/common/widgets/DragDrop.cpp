#include "compose/widgets/handler/DragDrop.h"

#include "compose/widgets/Widget.h"
#include <compose/input/TouchIndev.h>
#include "reactive/Deferrer.h"
#include "src/core/lv_obj_pos.h"
#include "src/misc/lv_async.h"

#include <algorithm>
#include <cassert>
#include <chrono>
#include <tools/Log.h>

namespace Compose
{
  namespace
  {
    constexpr int c_dragDetectionHysteresis = 20;
    constexpr int c_dragAxisDecisionHysteresis = 4;

    bool hasMultiTouch(lv_indev_t *indev)
    {
      return activeTouchCount(indev) > 1;
    }

    bool isPointInside(lv_obj_t *widget, const lv_point_t &point)
    {
      bool inside = false;

      if(!lv_obj_has_flag(widget, LV_OBJ_FLAG_HIDDEN))
      {
        lv_area_t coords;
        lv_obj_get_coords(widget, &coords);
        inside = point.x >= coords.x1 && point.x <= coords.x2 && point.y >= coords.y1 && point.y <= coords.y2;
      }

      return inside;
    }

    lv_obj_t *findWidgetAt(lv_obj_t *widget, const lv_point_t &point)
    {
      lv_obj_t *result = nullptr;

      if(!lv_obj_has_flag(widget, LV_OBJ_FLAG_HIDDEN))
      {
        const auto children = lv_obj_get_child_count(widget);

        for(uint32_t i = children; i > 0 && !result; --i)
        {
          if(auto *child = lv_obj_get_child(widget, i - 1))
          {
            result = findWidgetAt(child, point);
          }
        }

        if(!result && isPointInside(widget, point))
        {
          result = widget;
        }
      }

      return result;
    }

    std::string sourceKeyForType(const std::string &type)
    {
      return std::string("DragDropSource_") + type;
    }

    std::string targetKeyForType(const std::string &type)
    {
      return std::string("DragDropTarget_") + type;
    }

    struct DeferredDrop
    {
      DragDropContext::Setter setter;
      nlohmann::json content;
    };

    void runDeferredDrop(void *userData)
    {
      Reactive::Deferrer deferrer;
      const auto start = std::chrono::steady_clock::now();
      std::unique_ptr<DeferredDrop> drop(static_cast<DeferredDrop *>(userData));
      drop->setter(drop->content);
      const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
      if(duration >= std::chrono::milliseconds(20))
        Tools::Log::warn("Deferred drop callback took", duration.count(), "ms");
    }

    bool isDragStartAxisMatching(const DragDrop::DragDropForContent::Source::StartAxis::Direction axis, int xDiff, int yDiff)
    {
      const auto absX = std::abs(xDiff);
      const auto absY = std::abs(yDiff);
      auto isMatching = true;
      if(axis == DragDrop::DragDropForContent::Source::StartAxis::Horizontal)
      {
        isMatching = absX > absY;
      }
      else if(axis == DragDrop::DragDropForContent::Source::StartAxis::Vertical)
      {
        isMatching = absY > absX;
      }
      return isMatching;
    }

    void suppressScrollableAncestors(DragDrop::DragDropForContent::Source::Data *self)
    {
      if(!self->m_suppressedScrollables.empty())
      {
        return;
      }

      auto *candidate = lv_obj_get_parent(self->m_handle);
      while(candidate)
      {
        if(lv_obj_has_flag(candidate, LV_OBJ_FLAG_SCROLLABLE))
        {
          lv_obj_stop_scroll_anim(candidate);
          lv_obj_remove_flag(candidate, LV_OBJ_FLAG_SCROLLABLE);
          self->m_suppressedScrollables.push_back(candidate);
        }

        candidate = lv_obj_get_parent(candidate);
      }
    }

    void restoreScrollableAncestors(DragDrop::DragDropForContent::Source::Data *self)
    {
      for(auto *suppressed : self->m_suppressedScrollables)
      {
        if(suppressed && lv_obj_is_valid(suppressed))
        {
          lv_obj_add_flag(suppressed, LV_OBJ_FLAG_SCROLLABLE);
        }
      }

      self->m_suppressedScrollables.clear();
    }
  }

  DragDropContext &DragDropContext::get()
  {
    static DragDropContext instance;
    return instance;
  }

  void DragDropContext::setSource(lv_obj_t *self, const std::string &type, int offsetX, int offsetY, int rootX, int rootY, const Getter &getter,
                                  const DragWidgetBuilder &dragWidgetBuilder)
  {
    if(!m_source.get())
    {
      m_source = std::make_unique<Source>(self, type, offsetX, offsetY, rootX, rootY, getter, dragWidgetBuilder);
    }
  }

  void DragDropContext::resetSource(lv_obj_t *self)
  {
    if(auto *source = m_source.get().get())
    {
      if(source->m_widget == self)
      {
        std::unique_ptr<DeferredDrop> deferredDrop;

        if(source->m_currentTarget)
        {
          for(const auto &target : m_targets)
          {
            if(target->m_widget == source->m_currentTarget && target->m_type == source->m_type)
            {
              if(auto content = source->m_getter())
              {
                deferredDrop = std::make_unique<DeferredDrop>(DeferredDrop { target->m_setter, std::move(*content) });
              }

              break;
            }
          }
        }

        source->m_currentTarget = nullptr;
        m_source = nullptr;

        if(deferredDrop)
        {
          const auto res = lv_async_call(runDeferredDrop, deferredDrop.release());
          assert(res == LV_RESULT_OK);
        }
      }
    }
  }

  void DragDropContext::addTarget(lv_obj_t *self, const std::string &type, const Setter &setter)
  {
    m_targets.push_back(std::make_unique<Target>(self, type, setter));
  }

  void DragDropContext::removeTarget(lv_obj_t *self, const std::string &type)
  {
    std::erase_if(m_targets, [=](const auto &target) { return target->m_widget == self && target->m_type == type; });
  }

  void DragDropContext::onDragOver(lv_obj_t *dragSource, lv_obj_t *targetProspect, int rootX, int rootY)
  {
    if(auto *source = m_source.get().get())
    {
      if(source->m_widget == dragSource)
      {
        source->m_dragPosition = lv_point_t { rootX, rootY };

        if(source->m_dragWidget)
        {
          lv_obj_set_pos(source->m_dragWidget, rootX - source->m_dragWidgetOffset.first, rootY - source->m_dragWidgetOffset.second);
        }

        auto *candidate = targetProspect;
        lv_obj_t *currentTarget = nullptr;

        while(candidate && !currentTarget)
        {
          for(const auto &target : m_targets)
          {
            if(target->m_widget == candidate && target->m_type == source->m_type)
            {
              currentTarget = candidate;
              break;
            }
          }

          candidate = lv_obj_get_parent(candidate);
        }

        source->m_currentTarget = currentTarget;
      }
    }
  }

  bool DragDropContext::isCurrentTarget(lv_obj_t *widget) const
  {
    bool result = false;

    if(auto *source = m_source.get().get())
    {
      result = source->m_currentTarget == widget && widget != source->m_widget;
    }

    return result;
  }

  bool DragDropContext::isDragging() const
  {
    return m_source != nullptr;
  }

  std::optional<std::string> DragDropContext::getDraggedType() const
  {
    if(auto *source = m_source.get().get())
    {
      return source->m_type;
    }
    return {};
  }

  DragDropContext::Source::Source(lv_obj_t *widget, const std::string &type, int offsetX, int offsetY, int rootX, int rootY, const Getter &getter,
                                  const DragWidgetBuilder &dragWidgetBuilder)
      : m_widget(widget)
      , m_type(type)
      , m_getter(getter)
      , m_dragWidgetBuilder(dragWidgetBuilder)
      , m_dragPosition { rootX, rootY }
      , m_dragWidgetOffset { offsetX, offsetY }
  {
    m_dragWidget = lv_obj_create(lv_layer_top());
    lv_obj_add_flag(m_dragWidget, LV_OBJ_FLAG_FLOATING);
    lv_obj_add_flag(m_dragWidget, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_remove_flag(m_dragWidget, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(m_dragWidget, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_border_width(m_dragWidget, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(m_dragWidget, 0, LV_PART_MAIN);
    lv_obj_set_style_margin_all(m_dragWidget, 0, LV_PART_MAIN);

    if(m_dragWidgetBuilder)
    {
      lv_obj_set_size(m_dragWidget, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
      lv_obj_set_style_bg_opa(m_dragWidget, LV_OPA_TRANSP, LV_PART_MAIN);

      Widget w(m_dragWidget);
      m_dragWidgetBuilder(w);
      lv_obj_update_layout(m_dragWidget);
    }
    else
    {
      lv_area_t coords;
      lv_obj_get_coords(widget, &coords);
      const auto width = coords.x2 - coords.x1 + 1;
      const auto height = coords.y2 - coords.y1 + 1;
      lv_obj_set_size(m_dragWidget, width, height);
#if !LV_USE_SNAPSHOT
#error "LV_USE_SNAPSHOT must be enabled for default drag proxy rendering"
#else
      m_snapshot = lv_snapshot_take(widget, LV_COLOR_FORMAT_ARGB8888);
      assert(m_snapshot != nullptr);
      auto *image = lv_image_create(m_dragWidget);
      lv_image_set_src(image, m_snapshot);
      lv_obj_center(image);
      lv_obj_set_style_bg_opa(m_dragWidget, LV_OPA_TRANSP, LV_PART_MAIN);
#endif
    }

    lv_obj_set_pos(m_dragWidget, rootX - offsetX, rootY - offsetY);
  }

  DragDropContext::Source::~Source()
  {
    if(m_dragWidget && lv_obj_is_valid(m_dragWidget))
    {
      lv_obj_del(m_dragWidget);
      m_dragWidget = nullptr;
    }

    if(m_snapshot)
    {
      lv_draw_buf_destroy(m_snapshot);
      m_snapshot = nullptr;
    }
  }

  DragDropContext::Target::Target(lv_obj_t *widget, const std::string &type, const Setter &setter)
      : m_widget(widget)
      , m_type(type)
      , m_setter(setter)
  {
  }

  Drag::Data::Data(lv_obj_t *handle, const BeginCB &beginCB, const UpdateCB &updateCB, const EndCB &endCB)
      : m_handle(handle)
      , m_begin(beginCB)
      , m_update(updateCB)
      , m_end(endCB)
  {
    m_pressHandler = lv_obj_add_event_cb(
        m_handle,
        [](lv_event_t *e) {
          Reactive::Deferrer deferrer;
          if(auto *self = static_cast<Data *>(lv_event_get_user_data(e)))
          {
            if(auto *indev = lv_event_get_indev(e))
            {
              if(hasMultiTouch(indev))
              {
                return;
              }

              lv_point_t current;
              lv_indev_get_point(indev, &current);
              self->m_lastPos = current;
              self->m_begin();
            }
          }
        },
        LV_EVENT_PRESSED, this);

    m_pressingHandler = lv_obj_add_event_cb(
        m_handle,
        [](lv_event_t *e) {
          Reactive::Deferrer deferrer;
          if(auto *self = static_cast<Data *>(lv_event_get_user_data(e)))
          {
            if(auto *indev = lv_event_get_indev(e))
            {
              if(hasMultiTouch(indev))
              {
                if(self->m_lastPos)
                {
                  self->m_lastPos.reset();
                  self->m_end();
                }

                return;
              }

              lv_point_t current;
              lv_indev_get_point(indev, &current);

              if(self->m_lastPos)
              {
                self->m_update(current.x - self->m_lastPos->x, current.y - self->m_lastPos->y);
              }

              self->m_lastPos = current;
            }
          }
        },
        LV_EVENT_PRESSING, this);

    auto endDrag = [](lv_event_t *e) {
      Reactive::Deferrer deferrer;
      if(auto *self = static_cast<Data *>(lv_event_get_user_data(e)))
      {
        if(self->m_lastPos)
        {
          self->m_lastPos.reset();
          self->m_end();
        }
      }
    };

    m_releaseHandler = lv_obj_add_event_cb(m_handle, endDrag, LV_EVENT_RELEASED, this);
    m_pressLostHandler = lv_obj_add_event_cb(m_handle, endDrag, LV_EVENT_PRESS_LOST, this);
  }

  Drag::Data::~Data()
  {
    if(lv_obj_is_valid(m_handle))
    {
      lv_obj_remove_event_dsc(m_handle, m_pressHandler);
      lv_obj_remove_event_dsc(m_handle, m_pressingHandler);
      lv_obj_remove_event_dsc(m_handle, m_releaseHandler);
      lv_obj_remove_event_dsc(m_handle, m_pressLostHandler);
    }
  }

  Drag::Begin::Begin(Drag *self)
      : m_self(self)
  {
  }

  void Drag::Begin::operator<<(const BeginCB &cb) const
  {
    m_self->m_begin = cb;
  }

  Drag::Update::Update(Drag *self)
      : m_self(self)
  {
  }

  void Drag::Update::operator<<(const UpdateCB &cb) const
  {
    m_self->m_update = cb;
  }

  Drag::End::End(Drag *self)
      : m_self(self)
  {
  }

  void Drag::End::operator<<(const EndCB &cb) const
  {
    m_self->m_end = cb;
  }

  Drag::Drag(BaseWidget &w)
      : self(w)
      , begin(this)
      , update(this)
      , end(this)
  {
  }

  void Drag::operator<<(const std::function<void(Drag *it)> &cb)
  {
    cb(this);

    self.ensureDataForKeyExistsOwning<Data>("DragHandlerData", [this] { return new Data(self.getHandle(), m_begin, m_update, m_end); });
  }

  DragDrop::DragDropForContent::Source::Data::Data(lv_obj_t *handle, std::string type, const Getter &getter, const DragDropContext::DragWidgetBuilder &dragWidgetBuilder,
                                                   StartAxis::Direction startAxis)
      : m_handle(handle)
      , m_type(std::move(type))
      , m_getter(getter)
      , m_dragWidgetBuilder(dragWidgetBuilder)
      , m_startAxis(startAxis)
      , m_offset { 0, 0 }
  {
    m_pressHandler = lv_obj_add_event_cb(
        m_handle,
        [](lv_event_t *e) {
          Reactive::Deferrer deferrer;
          if(auto *self = static_cast<Data *>(lv_event_get_user_data(e)))
          {
            if(auto *indev = lv_event_get_indev(e))
            {
              if(hasMultiTouch(indev))
              {
                return;
              }

              lv_point_t point;
              lv_indev_get_point(indev, &point);
              self->m_startPos = point;
              self->m_startDecision = StartDecision::Undecided;
              restoreScrollableAncestors(self);

              lv_area_t widgetCoords;
              lv_obj_get_coords(self->m_handle, &widgetCoords);
              self->m_offset = lv_point_t { point.x - widgetCoords.x1, point.y - widgetCoords.y1 };
            }
          }
        },
        LV_EVENT_PRESSED, this);

    m_pressingHandler = lv_obj_add_event_cb(
        m_handle,
        [](lv_event_t *e) {
          Reactive::Deferrer deferrer;
          if(auto *self = static_cast<Data *>(lv_event_get_user_data(e)))
          {
            if(auto *indev = lv_event_get_indev(e))
            {
              if(hasMultiTouch(indev))
              {
                restoreScrollableAncestors(self);
                self->m_startPos.reset();
                self->m_startDecision = StartDecision::Undecided;
                auto *handle = self->m_handle;
                DragDropContext::get().resetSource(handle);
                return;
              }

              lv_point_t point;
              lv_indev_get_point(indev, &point);

              if(self->m_startPos)
              {
                const auto xDiff = point.x - self->m_startPos->x;
                const auto yDiff = point.y - self->m_startPos->y;
                const auto distance = std::sqrt(xDiff * xDiff + yDiff * yDiff);
                const auto isDragging = DragDropContext::get().isDragging();
                if(!isDragging && self->m_startDecision == StartDecision::Undecided && distance > c_dragAxisDecisionHysteresis)
                {
                  const auto hasMatchingStartAxis = isDragStartAxisMatching(self->m_startAxis, xDiff, yDiff);
                  self->m_startDecision = hasMatchingStartAxis ? StartDecision::AllowDrag : StartDecision::BlockDrag;
                  if(self->m_startDecision == StartDecision::AllowDrag && self->m_startAxis != StartAxis::Any)
                  {
                    suppressScrollableAncestors(self);
                  }
                }

                if(!isDragging && self->m_startDecision == StartDecision::AllowDrag && distance > c_dragDetectionHysteresis)
                {
                  DragDropContext::get().setSource(self->m_handle, self->m_type, self->m_offset.x, self->m_offset.y, point.x, point.y, self->m_getter, self->m_dragWidgetBuilder);
                }

                if(DragDropContext::get().isDragging())
                {
                  auto *targetWidget = findWidgetAt(lv_screen_active(), point);
                  DragDropContext::get().onDragOver(self->m_handle, targetWidget, point.x, point.y);
                }
              }
            }
          }
        },
        LV_EVENT_PRESSING, this);

    auto endDrag = [](lv_event_t *e) {
      Reactive::Deferrer deferrer;
      if(auto *self = static_cast<Data *>(lv_event_get_user_data(e)))
      {
        restoreScrollableAncestors(self);
        self->m_startPos.reset();
        self->m_startDecision = StartDecision::Undecided;
        auto *handle = self->m_handle;
        DragDropContext::get().resetSource(handle);
      }
    };

    m_releaseHandler = lv_obj_add_event_cb(m_handle, endDrag, LV_EVENT_RELEASED, this);
    m_pressLostHandler = lv_obj_add_event_cb(m_handle, endDrag, LV_EVENT_PRESS_LOST, this);
  }

  DragDrop::DragDropForContent::Source::Data::~Data()
  {
    restoreScrollableAncestors(this);

    if(lv_obj_is_valid(m_handle))
    {
      lv_obj_remove_event_dsc(m_handle, m_pressHandler);
      lv_obj_remove_event_dsc(m_handle, m_pressingHandler);
      lv_obj_remove_event_dsc(m_handle, m_releaseHandler);
      lv_obj_remove_event_dsc(m_handle, m_pressLostHandler);
    }
  }

  DragDrop::DragDropForContent::Source::Source(DragDropForContent *self)
      : self(self)
  {
  }

  void DragDrop::DragDropForContent::Source::setModifier(StartAxis axis)
  {
    m_startAxis = axis.it;

    const auto key = sourceKeyForType(self->type);
    BaseWidget owner(self->ownerHandle);
    if(auto *source = owner.getData<Data>(key))
    {
      source->m_startAxis = axis.it;
    }
  }

  void DragDrop::DragDropForContent::Source::operator<<(const Getter &cb) const
  {
    const auto key = sourceKeyForType(self->type);
    BaseWidget owner(self->ownerHandle);
    owner.ensureDataForKeyExistsOwning<Data>(key, [this, cb] { return new Data(self->ownerHandle, self->type, cb, self->buildDragWidget.get(), m_startAxis); });
  }

  DragDrop::DragDropForContent::Target::Target(DragDropForContent *self)
      : self(self)
  {
  }

  DragDrop::DragDropForContent::Target::Data::Data(lv_obj_t *handle, std::string type)
      : m_handle(handle)
      , m_type(std::move(type))
  {
    DragDropContext::get().addTarget(m_handle, m_type, [setter = m_setter](const nlohmann::json &content) { (*setter)(content); });
  }

  DragDrop::DragDropForContent::Target::Data::~Data()
  {
    DragDropContext::get().removeTarget(m_handle, m_type);
  }

  void DragDrop::DragDropForContent::Target::Data::setSetter(const Setter &setter)
  {
    *m_setter = setter;
  }

  void DragDrop::DragDropForContent::Target::operator<<(const Setter &cb)
  {
    const auto key = targetKeyForType(self->type);
    BaseWidget owner(self->ownerHandle);
    auto &data = owner.ensureDataForKeyExistsOwning<Data>(key, [this] { return new Data(self->ownerHandle, self->type); });
    data.setSetter(cb);
  }

  DragDrop::DragDropForContent::BuildDragWidget::BuildDragWidget(DragDropForContent *self)
      : self(self)
  {
  }

  void DragDrop::DragDropForContent::BuildDragWidget::operator<<(const Builder &cb)
  {
    m_builder = cb;

    const auto key = sourceKeyForType(self->type);
    BaseWidget owner(self->ownerHandle);
    if(auto *source = owner.getData<DragDrop::DragDropForContent::Source::Data>(key))
    {
      source->m_dragWidgetBuilder = cb;
    }
  }

  const DragDrop::DragDropForContent::BuildDragWidget::Builder &DragDrop::DragDropForContent::BuildDragWidget::get() const
  {
    return m_builder;
  }

  DragDrop::DragDropForContent::DragDropForContent(lv_obj_t *ownerHandle, std::string type)
      : ownerHandle(ownerHandle)
      , type(std::move(type))
      , buildDragWidget(this)
  {
  }

  void DragDrop::DragDropForContent::operator<<(const std::function<void(DragDropForContent *it)> &cb)
  {
    source = std::make_unique<Source>(this);
    target = std::make_unique<Target>(this);
    source->setModifier(m_startAxis);
    cb(this);
  }

  DragDrop::DragDrop(BaseWidget &w)
      : self(w)
  {
  }

  DragDrop::DragDropForContent &DragDrop::operator()(const std::string &type, DragDropForContent::Source::StartAxis startAxis)
  {
    if(!m_dragDropForContent->contains(type))
    {
      (*m_dragDropForContent)[type] = std::make_unique<DragDropForContent>(self.getHandle(), type);
    }

    auto &entry = *m_dragDropForContent->at(type);
    entry.m_startAxis = startAxis;
    if(entry.source)
    {
      entry.source->setModifier(startAxis);
    }
    return entry;
  }
}
