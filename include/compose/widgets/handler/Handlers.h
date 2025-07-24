#pragma once
#include "src/misc/lv_event_private.h"
#include <nltools/logging/Log.h>
#include <reactive/Deferrer.h>
#include <memory>
#include <assert.h>
#include <compose/widgets/BaseWidget.h>

namespace Compose
{
  struct LeftClick
  {
    BaseWidget &self;
    using CB = std::function<void()>;

    struct LeftClickData
    {
      LeftClickData(lv_obj_t *handle, const CB &cb)
          : m_handle(handle)
          , m_callback(cb)
      {
        m_handler = lv_obj_add_event_cb(
            handle,
            [](lv_event_t *e)
            {
              const auto user_data = static_cast<LeftClickData *>(lv_event_get_user_data(e));
              user_data->m_callback();
            },
            LV_EVENT_CLICKED, this);
      }

      ~LeftClickData()
      {
        lv_obj_remove_event_dsc(m_handle, m_handler);
      }

      lv_obj_t *m_handle;
      CB m_callback;
      lv_event_dsc_t *m_handler;
    };

    void operator<<(const CB &cb) const
    {
      assert(!self.getData<LeftClickData>(BaseWidget::c_leftClickKey));
      self.ensureDataForKeyExistsOwning<LeftClickData>(BaseWidget::c_leftClickKey,
                                                       [this, cb] { return new LeftClickData(self.getHandle(), cb); });
    }
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
      self.ensureDataForKeyExistsOwning<StateChangeData>(BaseWidget::c_stateChangeKey, [this, cb]
                                                         { return new StateChangeData(self.getHandle(), cb); });
    }
  };
}
