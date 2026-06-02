#pragma once
#include "src/misc/lv_event_private.h"
#include "src/misc/lv_area.h"
#include "src/core/lv_obj_event.h"
#include "src/core/lv_obj_pos.h"
#include "src/core/lv_obj.h"

#include <reactive/Deferrer.h>
#include <assert.h>
#include <compose/widgets/BaseWidget.h>
#include <compose/modifiers/Position.h>
#include <compose/input/TouchIndev.h>

#include <optional>
#include <type_traits>
#include <unordered_map>
#include <utility>

namespace Compose
{
  namespace Detail
  {
    struct TouchEvent
    {
      uint32_t pointerId = 0;
      Position position;
      size_t activeTouchCount = 0;
    };

    inline std::optional<TouchEvent> getTouchEvent(lv_event_t *e, lv_obj_t *handle)
    {
      if(const auto indev = lv_event_get_indev(e))
      {
        const auto *rawDriverData = lv_indev_get_driver_data(indev);
        const auto *touchData = getTouchIndevData(indev);
        if(!touchData)
        {
          return std::nullopt;
        }

        const auto pointerId = touchData->pointerId;
        lv_point_t point = { 0, 0 };
        lv_indev_get_point(indev, &point);
        lv_area_t widgetCoords;
        lv_obj_get_coords(handle, &widgetCoords);
        return TouchEvent {
          .pointerId = pointerId,
          .position = { point.x - widgetCoords.x1, point.y - widgetCoords.y1 },
          .activeTouchCount = activeTouchCount(indev),
        };
      }

      return std::nullopt;
    }
  }

  template <lv_event_code_t FilterType> struct ClickHandler
  {
    const char *m_key;
    BaseWidget &self;
    using CB = std::function<bool(Position)>;

    ClickHandler(BaseWidget &w, auto key)
        : m_key { key }
        , self { w }
    {
    }

    struct ClickData
    {
      ClickData(lv_obj_t *handle, const CB &cb)
          : m_handle(handle)
          , m_callback(cb)
      {
        m_handler = lv_obj_add_event_cb(
            handle,
            [](lv_event_t *e)
            {
              Reactive::Deferrer def;
              const auto user_data = static_cast<ClickData *>(lv_event_get_user_data(e));
              const auto indev = lv_event_get_indev(e);
              if(indev)
              {
                lv_point_t point = { 0, 0 };
                lv_indev_get_point(indev, &point);
                lv_area_t widget_coords;
                lv_obj_get_coords(user_data->m_handle, &widget_coords);
                const int rel_x = point.x - widget_coords.x1;
                const int rel_y = point.y - widget_coords.y1;
                e->stop_processing = user_data->m_callback({ rel_x, rel_y });
              }
            },
            FilterType, this);
      }

      ~ClickData()
      {
        lv_obj_remove_event_dsc(m_handle, m_handler);
      }

      lv_obj_t *m_handle;
      CB m_callback;
      lv_event_dsc_t *m_handler;
    };

    void operator<<(const CB &cb) const
    {
      assert(!self.getData<ClickData>(m_key));
      self.ensureDataForKeyExistsOwning<ClickData>(m_key, [this, cb] { return new ClickData(self.getHandle(), cb); });
    }
  };

  using LeftClick = ClickHandler<LV_EVENT_CLICKED>;
  using LongClick = ClickHandler<LV_EVENT_LONG_PRESSED>;

  struct Touch
  {
    BaseWidget &self;
    using CB = std::function<void(uint32_t, Position, size_t)>;

    template <typename tCB> static CB makeCallback(tCB &&cb)
    {
      return [cb = std::forward<tCB>(cb)](uint32_t pointerId, Position position, size_t activeTouchCount) mutable
      {
        if constexpr(std::is_invocable_v<tCB, uint32_t, Position, size_t>)
        {
          cb(pointerId, position, activeTouchCount);
        }
        else if constexpr(std::is_invocable_v<tCB, uint32_t, Position>)
        {
          cb(pointerId, position);
        }
        else if constexpr(std::is_invocable_v<tCB, Position>)
        {
          cb(position);
        }
        else if constexpr(std::is_invocable_v<tCB, uint32_t>)
        {
          cb(pointerId);
        }
        else if constexpr(std::is_invocable_v<tCB>)
        {
          cb();
        }
        else
        {
          static_assert(std::is_invocable_v<tCB, uint32_t, Position, size_t> || std::is_invocable_v<tCB, uint32_t, Position> || std::is_invocable_v<tCB, Position>
                            || std::is_invocable_v<tCB, uint32_t> || std::is_invocable_v<tCB>,
                        "Touch callback must be invocable with (uint32_t, Position, size_t), (uint32_t, Position), (Position), (uint32_t) or ()");
        }
      };
    }

    struct Data
    {
      Data(lv_obj_t *handle, const CB &beginCB, const CB &updateCB, const CB &endCB)
          : m_handle(handle)
          , m_begin(beginCB)
          , m_update(updateCB)
          , m_end(endCB)
      {
        m_pressedHandler = lv_obj_add_event_cb(
            handle,
            [](lv_event_t *e)
            {
              Reactive::Deferrer def;
              if(auto *self = static_cast<Data *>(lv_event_get_user_data(e)))
              {
                if(auto touchEvent = Detail::getTouchEvent(e, self->m_handle))
                {
                  const auto [it, inserted] = self->m_activePointers.insert_or_assign(touchEvent->pointerId, touchEvent->position);
                  if(inserted)
                  {
                    self->m_begin(touchEvent->pointerId, touchEvent->position, touchEvent->activeTouchCount);
                  }
                }
              }
            },
            LV_EVENT_PRESSED, this);

        m_pressingHandler = lv_obj_add_event_cb(
            handle,
            [](lv_event_t *e)
            {
              Reactive::Deferrer def;
              if(auto *self = static_cast<Data *>(lv_event_get_user_data(e)))
              {
                if(auto touchEvent = Detail::getTouchEvent(e, self->m_handle))
                {
                  self->m_activePointers[touchEvent->pointerId] = touchEvent->position;
                  self->m_update(touchEvent->pointerId, touchEvent->position, touchEvent->activeTouchCount);
                }
              }
            },
            LV_EVENT_PRESSING, this);

        auto onRelease = [](lv_event_t *e)
        {
          Reactive::Deferrer def;
          if(auto *self = static_cast<Data *>(lv_event_get_user_data(e)))
          {
            if(auto touchEvent = Detail::getTouchEvent(e, self->m_handle))
            {
              if(self->m_activePointers.erase(touchEvent->pointerId) > 0)
              {
                self->m_end(touchEvent->pointerId, touchEvent->position, touchEvent->activeTouchCount);
              }
            }
          }
        };

        m_releasedHandler = lv_obj_add_event_cb(handle, onRelease, LV_EVENT_RELEASED, this);
        m_pressLostHandler = lv_obj_add_event_cb(handle, onRelease, LV_EVENT_PRESS_LOST, this);
      }

      ~Data()
      {
        for(const auto &[pointerId, position] : m_activePointers)
        {
          m_end(pointerId, position, 0);
        }

        m_activePointers.clear();

        if(lv_obj_is_valid(m_handle))
        {
          lv_obj_remove_event_dsc(m_handle, m_pressedHandler);
          lv_obj_remove_event_dsc(m_handle, m_pressingHandler);
          lv_obj_remove_event_dsc(m_handle, m_releasedHandler);
          lv_obj_remove_event_dsc(m_handle, m_pressLostHandler);
        }
      }

      lv_obj_t *m_handle;
      CB m_begin;
      CB m_update;
      CB m_end;
      std::unordered_map<uint32_t, Position> m_activePointers;
      lv_event_dsc_t *m_pressedHandler = nullptr;
      lv_event_dsc_t *m_pressingHandler = nullptr;
      lv_event_dsc_t *m_releasedHandler = nullptr;
      lv_event_dsc_t *m_pressLostHandler = nullptr;
    };

    struct Begin
    {
      Touch *self;

      template <typename tCB> void operator<<(tCB &&cb) const
      {
        self->m_begin = makeCallback(std::forward<tCB>(cb));
      }
    };

    struct Update
    {
      Touch *self;

      template <typename tCB> void operator<<(tCB &&cb) const
      {
        self->m_update = makeCallback(std::forward<tCB>(cb));
      }
    };

    struct End
    {
      Touch *self;

      template <typename tCB> void operator<<(tCB &&cb) const
      {
        self->m_end = makeCallback(std::forward<tCB>(cb));
      }
    };

    void operator<<(const std::function<void(Touch *it)> &cb)
    {
      cb(this);
      self.ensureDataForKeyExistsOwning<Data>("TouchData", [this] { return new Data(self.getHandle(), m_begin, m_update, m_end); });
    }

    CB m_begin = [](uint32_t, Position, size_t) { };
    CB m_update = [](uint32_t, Position, size_t) { };
    CB m_end = [](uint32_t, Position, size_t) { };
    Begin begin { this };
    Update update { this };
    End end { this };
  };

  struct StateChange
  {
    BaseWidget &self;
    using CB = std::function<void(bool)>;

    struct StateChangeData
    {
      StateChangeData(lv_obj_t *handle, const CB &cb)
          : m_handle { handle }
          , m_callback(cb)
      {
        m_handler = lv_obj_add_event_cb(
            handle,
            [](lv_event_t *e)
            {
              Reactive::Deferrer def;
              const auto user_data = static_cast<StateChangeData *>(lv_event_get_user_data(e));
              const auto checked = lv_obj_has_state(user_data->m_handle, LV_STATE_CHECKED);
              user_data->m_callback(checked);
            },
            LV_EVENT_VALUE_CHANGED, this);
      }

      ~StateChangeData()
      {
        lv_obj_remove_event_dsc(m_handle, m_handler);
      }

      lv_obj_t *m_handle;
      CB m_callback;
      lv_event_dsc_t *m_handler;
    };

    void operator<<(const CB &cb) const
    {
      assert(!self.getData<StateChangeData>(BaseWidget::c_stateChangeKey));
      self.ensureDataForKeyExistsOwning<StateChangeData>(BaseWidget::c_stateChangeKey, [this, cb] { return new StateChangeData(self.getHandle(), cb); });
    }
  };

  struct TimerTick
  {
    BaseWidget &self;
    using CB = std::function<void()>;

    struct TimerData
    {
      lv_timer_t *timer;
      CB cb;

      static void callback(lv_timer_t *timer)
      {
        auto self = static_cast<TimerData *>(lv_timer_get_user_data(timer));
        self->cb();
      }

      TimerData(lv_obj_t *handle, const CB &cb)
          : cb(cb)
      {
        timer = lv_timer_create(callback, 16, this);
      }

      ~TimerData()
      {
        lv_timer_del(timer);
      }
    };

    void operator<<(const CB &cb) const
    {
      self.ensureDataForKeyExistsOwning<TimerData>("TimerData", [this, cb] { return new TimerData(self.getHandle(), cb); });
    }
  };
}
