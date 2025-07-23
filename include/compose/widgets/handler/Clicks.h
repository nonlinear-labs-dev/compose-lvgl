#pragma once
#include "src/misc/lv_event_private.h"

#include <nltools/logging/Log.h>
#include <reactive/Deferrer.h>
#include <memory>
#include <compose/widgets/BaseWidget.h>

namespace Compose
{
  template <typename tHandle> class LeftClick : public std::enable_shared_from_this<LeftClick<tHandle>>
  {
   public:
    tHandle& widget;
    std::function<void()> m_callback;

    lv_event_dsc_t* debugDesc;

    lv_obj_t* widgetPtr;

    explicit LeftClick(tHandle& widget)
        : widget(widget)
    {
      auto handle = widget.getHandle();
      widgetPtr = handle;
      debugDesc = lv_obj_add_event_cb(
          handle,
          [](lv_event_t* e)
          {
            if(lv_event_get_code(e) == LV_EVENT_CLICKED)
            {
              const auto target = static_cast<lv_obj_t*>(lv_event_get_target(e));
              nltools::Log::error(std::format("Our ID: {}", BaseWidget(target).getID()));
              if(const auto parent = lv_obj_get_parent(target))
              {
                nltools::Log::error(std::format("Parent ID: {}", BaseWidget(parent).getID()));
              }
            }
          },
          LV_EVENT_ALL, this);
    }

    ~LeftClick()
    {
      lv_obj_remove_event_dsc(widgetPtr, debugDesc);
    }

    template <typename tCB> void operator<<(const tCB& cb)
    {
      m_callback = cb;
      auto handle = widget.getHandle();
      lv_obj_add_event_cb(
          handle,
          [](lv_event_t* e)
          {
            auto user_data = static_cast<LeftClick*>(lv_event_get_user_data(e));

            if(lv_event_get_code(e) == LV_EVENT_CLICKED)
            {
              if(user_data->m_callback)
              {
                user_data->m_callback();
              }
            }
          },
          LV_EVENT_ALL, this);

      struct SharedPtrWrapper
      {
        std::shared_ptr<LeftClick> handler_ptr;
      };

      auto* wrapper = new SharedPtrWrapper(LeftClick::shared_from_this());

      lv_obj_add_event_cb(
          handle,
          [](lv_event_t* e)
          {
            if(lv_event_get_code(e) == LV_EVENT_DELETE)
            {
              delete static_cast<SharedPtrWrapper*>(lv_event_get_user_data(e));
            }
          },
          LV_EVENT_DELETE, wrapper);
    }
  };

  template <typename tHandle> class StateChange : public std::enable_shared_from_this<StateChange<tHandle>>
  {
   public:
    tHandle& widget;
    std::function<void(bool)> m_callback;

    explicit StateChange(tHandle& widget)
        : widget(widget)
    {
    }

    template <typename tCB> void operator<<(const tCB& cb)
    {
      m_callback = cb;
      auto handle = widget.getHandle();
      lv_obj_add_event_cb(
          handle,
          [](lv_event_t* e)
          {
            auto user_data = static_cast<StateChange*>(lv_event_get_user_data(e));
            if(lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED)
            {
              if(user_data->m_callback)
              {
                auto obj = static_cast<lv_obj_t*>(lv_event_get_target(e));
                bool checked = lv_obj_has_state(obj, LV_STATE_CHECKED);
                user_data->m_callback(checked);
              }
            }
          },
          LV_EVENT_ALL, this);

      struct SharedPtrWrapper
      {
        std::shared_ptr<StateChange> handler_ptr;
      };

      auto* wrapper = new SharedPtrWrapper(StateChange::shared_from_this());

      lv_obj_add_event_cb(
          handle,
          [](lv_event_t* e)
          {
            if(lv_event_get_code(e) == LV_EVENT_DELETE)
            {
              delete static_cast<SharedPtrWrapper*>(lv_event_get_user_data(e));
            }
          },
          LV_EVENT_DELETE, wrapper);
    }
  };
}

#define LEFT_CLICK() *it.leftClickHandler << [=]
#define STATE_CHANGE *it.stateChangeHandler << [=]