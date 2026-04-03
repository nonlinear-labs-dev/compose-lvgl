#pragma once
#include "src/misc/lv_event_private.h"
#include "src/misc/lv_area.h"
#include "src/core/lv_obj_event.h"
#include "src/core/lv_obj_pos.h"
#include "tools/Log.h"

#include <reactive/Deferrer.h>
#include <assert.h>
#include <compose/widgets/BaseWidget.h>
#include <compose/modifiers/Position.h>

namespace Compose
{

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
            [](lv_event_t *e) {
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

  using LeftClick = ClickHandler<LV_EVENT_SHORT_CLICKED>;
  using LongClick = ClickHandler<LV_EVENT_LONG_PRESSED>;

  struct Touch
  {
    BaseWidget &self;
    using CB = std::function<void()>;

    struct Data
    {
      Data(lv_obj_t *handle, const CB &beginCB, const CB &endCB)
          : m_handle(handle)
          , m_begin(beginCB)
          , m_end(endCB)
      {
        m_pressedHandler = lv_obj_add_event_cb(
            handle,
            [](lv_event_t *e) {
              Reactive::Deferrer def;
              if(auto *self = static_cast<Data *>(lv_event_get_user_data(e)))
              {
                if(!self->m_pressed)
                {
                  self->m_pressed = true;
                  Tools::Log::warn("Touch pressed", self->m_handle);
                  self->m_begin();
                }
              }
            },
            LV_EVENT_PRESSED, this);

        auto onRelease = [](lv_event_t *e) {
          Reactive::Deferrer def;
          if(auto *self = static_cast<Data *>(lv_event_get_user_data(e)))
          {
            if(self->m_pressed)
            {
              Tools::Log::warn("Touch released", self->m_handle);
              self->m_pressed = false;
              self->m_end();
            }
          }
        };

        m_releasedHandler = lv_obj_add_event_cb(handle, onRelease, LV_EVENT_RELEASED, this);
        m_pressLostHandler = lv_obj_add_event_cb(handle, onRelease, LV_EVENT_PRESS_LOST, this);
      }

      ~Data()
      {
        if(m_pressed)
        {
          Tools::Log::warn("Touch released (destroyed)", m_handle);
          m_pressed = false;
          m_end();
        }

        if(lv_obj_is_valid(m_handle))
        {
          lv_obj_remove_event_dsc(m_handle, m_pressedHandler);
          lv_obj_remove_event_dsc(m_handle, m_releasedHandler);
          lv_obj_remove_event_dsc(m_handle, m_pressLostHandler);
        }
      }

      lv_obj_t *m_handle;
      CB m_begin;
      CB m_end;
      bool m_pressed = false;
      lv_event_dsc_t *m_pressedHandler = nullptr;
      lv_event_dsc_t *m_releasedHandler = nullptr;
      lv_event_dsc_t *m_pressLostHandler = nullptr;
    };

    struct Begin
    {
      Touch *self;
      void operator<<(const CB &cb) const
      {
        self->m_begin = cb;
      }
    };

    struct End
    {
      Touch *self;
      void operator<<(const CB &cb) const
      {
        self->m_end = cb;
      }
    };

    void operator<<(const std::function<void(Touch *it)> &cb)
    {
      cb(this);
      self.ensureDataForKeyExistsOwning<Data>("TouchData", [this] { return new Data(self.getHandle(), m_begin, m_end); });
    }

    CB m_begin = [] {};
    CB m_end = [] {};
    Begin begin { this };
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
            [](lv_event_t *e) {
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
}
